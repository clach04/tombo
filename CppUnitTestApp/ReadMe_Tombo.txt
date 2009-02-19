Unit test app for TOMBO sources.

Most of the file under this directory is a part of cppunit-1.10.2 except folder "test".

For details about cppunit, please see :
http://cppunit.sourceforge.net/cppunit-wiki

To test using by CppUnitTestApp, you have to :
1. install cppunit.
2. Add include/library path to project.
3. Place testrunnerd.dll under the "Debug" folder.

Tombo/
 +- BE300/
 +- cppunit/
 |   +- lib/
 |   |   +- cppunitd.lib, testrunnerd.lib
 |   +- include/
 |       +- cppunit/
 |            +- (config/, extensions/, Test.h, TestAssert.h , ... 
 +- CppUnitTestApp
   ....


hirami
