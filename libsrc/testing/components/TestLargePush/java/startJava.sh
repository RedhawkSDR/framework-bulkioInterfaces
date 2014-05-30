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

# Path for Java
if test -x $JAVA_HOME/bin/java; then
  JAVA=$JAVA_HOME/bin/java
else
  JAVA=java
fi

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
exec $JAVA_HOME/bin/java -cp ::$myDir/TestLargePush.jar:$myDir/bin:$bulkio_libsrc_top/bulkio.jar:$bulkio_top/BULKIOInterfaces.jar:$CLASSPATH TestLargePush.java.TestLargePush "$@"
#exec $JAVA -cp :$myDir/TestLargePush.jar:$myDir/bin:$CLASSPATH TestLargePush.java.TestLargePush "$@"

#JacORB start lines
#exec $JAVA -cp :$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/TestLargePush.jar:$myDir/bin:$CLASSPATH TestLargePush.java.TestLargePush "$@"
