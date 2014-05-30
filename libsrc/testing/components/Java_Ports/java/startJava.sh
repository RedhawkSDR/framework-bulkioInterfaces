#!/bin/sh
myDir=`dirname $0`

#
# this needs to be absolute path to python unit test framework will work
#
bulkio_top=$myDir/../../../../../
bulkio_libsrc_top=$bulkio_top/libsrc

# Setup the OSSIEHOME Lib jars on the classpath
libDir=$OSSIEHOME/lib
libFiles=`ls -1 $libDir/*.jar`
for file in $libFiles
do
	if [ x"$CLASSPATH" = "x" ]
	then
		export CLASSPATH=$file
	else
		export CLASSPATH=$file:$CLASSPATH
	fi
done

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
# JNI
exec $JAVA_HOME/bin/java -cp ::$myDir/Java_Ports.jar:$myDir/bin:$bulkio_libsrc_top/bulkio.jar:$bulkio_top/BULKIOInterfaces.jar:$CLASSPATH Java_Ports.java.Java_Ports "$@"
#exec $JAVA_HOME/bin/java -cp ::$myDir/Java_Ports.jar:$myDir/bin:$CLASSPATH Java_Ports.java.Java_Ports "$@"

#JacORB start lines
#$JAVA_HOME/bin/java -cp ::$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/TestJava.jar:$myDir/bin:$CLASSPATH TestJava.java.TestJava "$@"
