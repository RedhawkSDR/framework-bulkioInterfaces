/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "bulkio_in_stream.h"
#include "bulkio_time_operators.h"
#include "bulkio_in_port.h"

namespace {
  template <class T, class Alloc>
  class stealable_vector : public std::vector<T, Alloc> {
  public:
    stealable_vector()
    {
    }

    T* steal()
    {
      T* out = this->_M_impl._M_start;
      this->_M_impl._M_start = 0;
      this->_M_impl._M_finish = 0;
      this->_M_impl._M_end_of_storage = 0;
      return out;
    }
  };

  template <class T, class Alloc>
  T* steal_buffer(std::vector<T,Alloc>& vec)
  {
    stealable_vector<T,Alloc> other;
    std::swap(vec, other);
    return other.steal();
  }
}

using bulkio::InputStream;

template <class PortTraits>
class InputStream<PortTraits>::Impl {
public:
  typedef PortTraits TraitsType;
  typedef DataTransfer<typename TraitsType::DataTransferTraits> DataTransferType;
  typedef typename DataTransferType::NativeDataType NativeType;
  typedef std::vector<NativeType> VectorType;
  typedef DataBlock<NativeType> DataBlockType;

  Impl(const BULKIO::StreamSRI& sri, bulkio::InPort<PortTraits>* port) :
    _streamID(sri.streamID),
    _sri(sri),
    _eosReceived(false),
    _eosReached(false),
    _port(port),
    _queue(),
    _pending(0),
    _samplesQueued(0),
    _sampleOffset(0),
    _enabled(true)
  {
  }

  ~Impl()
  {
    delete _pending;
  }

  const std::string& streamID() const
  {
    return _streamID;
  }

  const BULKIO::StreamSRI& sri() const
  {
    return _sri;
  }

  bool eos() const {
    return _eosReached;
  }

  size_t samplesAvailable()
  {
    // Peek at the next SRI, which may move a pending packet into the read
    // queue, but will not try to fetch a packet from the port
    int item_size = _peekSRI().mode?2:1;

    // If the queue is empty, this is the first read of a segment (i.e., search
    // can go past the first packet if the SRI change or queue flush flag is
    // set)
    bool first = _queue.empty();
    size_t queued = _samplesQueued / item_size;
    if (_pending) {
      // There is an SRI change or input queue flush; the stream queue cannot
      // be empty, because the peek would have moved the pending packet onto
      // the read queue, so just return what is queued
      return queued;
    }

    return queued + _port->samplesAvailable(_streamID, first);
  }

  DataBlockType readPacket()
  {
    if (_samplesQueued == 0) {
      _fetchPacket();
    }

    if (_samplesQueued == 0) {
      return DataBlockType();
    }
    const size_t samples = _queue.front()->dataBuffer.size() - _sampleOffset;
    return _readData(samples, samples);
  }

  DataBlockType read(size_t count, size_t consume)
  {
    // If the next block of data is complex, double the read and consume size
    // (which the lower-level I/O handles in terms of scalars) so that the
    // returned block has the right number of samples
    if (_nextSRI().mode == 1) {
      count *= 2;
      consume *= 2;
    }

    // Queue up packets from the port until we have enough data to satisfy the
    // requested read amount
    while (_samplesQueued < count) {
      if (!_fetchPacket()) {
        break;
      }
    }

    if (_samplesQueued == 0) {
      return DataBlockType();
    }

    // Only read as many samples as are available (e.g., if a new SRI is coming
    // or the stream reached the end)
    const size_t samples = std::min(count, _samplesQueued);

    // If the read pointer has reached the end of a segment (new SRI, queue
    // flush or end-of-stream), consume all remaining data
    if (samples < count) {
      consume = samples;
    }

    return _readData(samples, consume);
  }

  size_t skip(size_t count)
  {
    // If the next block of data is complex, double the skip size (which the
    // lower-level I/O handles in terms of scalars) so that the right number of
    // samples is skipped
    size_t item_size = _nextSRI().mode?2:1;
    count *= item_size;

    // Queue up packets from the port until we have enough data to satisfy the
    // requested read amount
    while (_samplesQueued < count) {
      if (!_fetchPacket()) {
        break;
      }
    }

    count = std::min(count, _samplesQueued);
    _consumeData(count);

    // Convert scalars back to samples
    return count / item_size;
  }

  bool ready()
  {
    if (!_enabled) {
      return false;
    } else if (_samplesQueued) {
      return true;
    } else {
      return samplesAvailable() > 0;
    }
  }

  bool enabled() const
  {
    return _enabled;
  }

  void enable()
  {
    _enabled = true;
  }

  void disable()
  {
    _enabled = false;
    // TODO: purge queue
  }

private:
  void _consumeData(size_t count)
  {
    while (count > 0) {
      const VectorType& data = _queue.front()->dataBuffer;

      const size_t available = data.size() - _sampleOffset;
      const size_t pass = std::min(available, count);

      _sampleOffset += pass;
      _samplesQueued -= pass;
      count -= pass;

      if (_sampleOffset >= data.size()) {
        // Read pointer has passed the end of the packet data
        _consumePacket();
        _sampleOffset = 0;
      }
    }
  }

  void _consumePacket()
  {
    // Acknowledge any end-of-stream flag and delete the packet
    DataTransferType* packet = _queue.front();
    _eosReached = packet->EOS;
    // The packet buffer was allocated with new[] by the CORBA layer, while
    // vector will use non-array delete, so explicitly delete the buffer
    delete[] steal_buffer(packet->dataBuffer);
    delete packet;
    _queue.erase(_queue.begin());
  }

  DataBlockType _readData(size_t count, size_t consume)
  {
    // Acknowledge pending SRI change
    DataTransferType* front = _queue.front();
    int sriChangeFlags = bulkio::sri::NONE;
    if (front->sriChanged) {
      sriChangeFlags = bulkio::sri::compareFields(_sri, front->SRI);
      front->sriChanged = false;
      _sri = front->SRI;
    }

    // Allocate empty data block and propagate the SRI change and input queue
    // flush flags
    DataBlockType data(_sri);
    data.sriChangeFlags(sriChangeFlags);
    if (front->inputQueueFlushed) {
      data.inputQueueFlushed(true);
      front->inputQueueFlushed = false;
    }

    if ((count <= consume) && (_sampleOffset == 0) && (front->dataBuffer.size() == count)) {
      // Optimization: when the read aligns perfectly with the front packet's
      // data buffer, and the entire packet is being consumed, swap the vector
      // data
      data.addTimestamp(bulkio::SampleTimestamp(front->T, 0));
      data.swap(front->dataBuffer);
      _samplesQueued -= count;
      _consumePacket();
      return data;
    }

    data.resize(count);
    NativeType* data_buffer = data.data();
    size_t data_offset = 0;

    // Assemble data that may span several input packets into the output buffer
    size_t packet_index = 0;
    size_t packet_offset = _sampleOffset;
    while (count > 0) {
      DataTransferType* packet = _queue[packet_index];
      const VectorType& input_data = packet->dataBuffer;

      // Determine the timestamp of this chunk of data; if this is the
      // first chunk, the packet offset (number of samples already read)
      // must be accounted for, so adjust the timestamp based on the SRI.
      // Otherwise, the adjustment is a noop.
      BULKIO::PrecisionUTCTime time = packet->T;
      double time_offset = packet_offset * packet->SRI.xdelta;
      size_t sample_offset = data_offset;
      if (packet->SRI.mode) {
        // Complex data; each sample is two values
        time_offset /= 2.0;
        sample_offset /= 2;
      }

      // If there is a time offset, apply the adjustment and mark the timestamp
      // so that the caller knows it was calculated rather than received
      bool synthetic = false;
      if (time_offset > 0.0) {
        time += time_offset;
        synthetic = true;
      }

      data.addTimestamp(bulkio::SampleTimestamp(time, sample_offset, synthetic));

      // The number of samples copied on this pass may be less than the total
      // remaining
      const size_t available = input_data.size() - packet_offset;
      const size_t pass = std::min(available, count);

      std::copy(&input_data[packet_offset], &input_data[packet_offset+pass], &data_buffer[data_offset]);
      data_offset += pass;
      packet_offset += pass;
      count -= pass;

      // If all the data from the current packet has been read, move on to
      // the next
      if (packet_offset >= input_data.size()) {
        packet_offset = 0;
        ++packet_index;
      }
    }

    // Advance the read pointers
    _consumeData(consume);

    return data;
  }

  const BULKIO::StreamSRI& _nextSRI()
  {
    if (_queue.empty()) {
      if (!_fetchPacket()) {
        // The last packet has been consumed, return the cached SRI
        return _sri;
      }
    }

    return _queue.front()->SRI;
  }

  const BULKIO::StreamSRI& _peekSRI()
  {
    if (_queue.empty()) {
      if (_pending) {
        _queuePacket(_pending);
        _pending = 0;
      } else {
        return _sri;
      }
    }
    return _queue.front()->SRI;
  }

  bool _fetchPacket()
  {
    if (_pending) {
      if (_queue.empty()) {
        // There are no packets currently queued, move pending packet onto the
        // read queue
        _queuePacket(_pending);
        _pending = 0;
        return true;
      } else {
        // Cannot read another packet until non-bridging packet is acknowledged
        return false;
      }
    }

    // Any future packets with this stream ID belong to another InputStream
    if (_eosReceived) {
      return false;
    }

    DataTransferType* packet = _port->getPacket(bulkio::Const::BLOCKING, _streamID);
    if (!packet) {
      return false;
    }

    _eosReceived = packet->EOS;
    if (_queue.empty() || _canBridge(packet)) {
      _queuePacket(packet);
      return true;
    } else {
      _pending = packet;
      return false;
    }
  }

  void _queuePacket(DataTransferType* packet)
  {
    _samplesQueued += packet->dataBuffer.size();
    _queue.push_back(packet);
  }

  bool _canBridge(DataTransferType* packet) const
  {
    return !(packet->sriChanged || packet->inputQueueFlushed);
  }

  const std::string _streamID;
  BULKIO::StreamSRI _sri;
  bool _eosReceived;
  bool _eosReached;
  InPort<PortTraits>* _port;
  std::vector<DataTransferType*> _queue;
  DataTransferType* _pending;
  size_t _samplesQueued;
  size_t _sampleOffset;
  bool _enabled;
};


template <class PortTraits>
InputStream<PortTraits>::InputStream() :
  _impl()
{
}

template <class PortTraits>
InputStream<PortTraits>::InputStream(const BULKIO::StreamSRI& sri, bulkio::InPort<PortTraits>* port) :
  _impl(new Impl(sri, port))
{
}

template <class PortTraits>
const std::string& InputStream<PortTraits>::streamID() const
{
  return _impl->streamID();
}

template <class PortTraits>
const BULKIO::StreamSRI& InputStream<PortTraits>::sri() const
{
  return _impl->sri();
}

template <class PortTraits>
bool InputStream<PortTraits>::eos() const
{
  return _impl->eos();
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read()
{
  return _impl->readPacket();
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read(size_t count)
{
  return _impl->read(count, count);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read(size_t count, size_t consume)
{
  return _impl->read(count, consume);
}

template <class PortTraits>
size_t InputStream<PortTraits>::skip(size_t count)
{
  return _impl->skip(count);
}

template <class PortTraits>
bool InputStream<PortTraits>::enabled() const
{
  return _impl->enabled();
}

template <class PortTraits>
void InputStream<PortTraits>::enable()
{
  _impl->enable();
}

template <class PortTraits>
void InputStream<PortTraits>::disable()
{
  _impl->disable();
}

template <class PortTraits>
size_t InputStream<PortTraits>::samplesAvailable()
{
  return _impl->samplesAvailable();
}

template <class PortTraits>
bool InputStream<PortTraits>::operator!() const
{
  return !_impl;
}

template <class PortTraits>
bool InputStream<PortTraits>::operator==(const InputStream& other) const
{
  return _impl.get() == other._impl.get();
}

template <class PortTraits>
bool InputStream<PortTraits>::ready()
{
  return _impl->ready();
}

template class InputStream<bulkio::CharPortTraits>;
template class InputStream<bulkio::OctetPortTraits>;
template class InputStream<bulkio::ShortPortTraits>;
template class InputStream<bulkio::UShortPortTraits>;
template class InputStream<bulkio::LongPortTraits>;
template class InputStream<bulkio::ULongPortTraits>;
template class InputStream<bulkio::LongLongPortTraits>;
template class InputStream<bulkio::ULongLongPortTraits>;
template class InputStream<bulkio::FloatPortTraits>;
template class InputStream<bulkio::DoublePortTraits>;
