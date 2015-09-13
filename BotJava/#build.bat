@echo off
javac -cp ".;tyrus-standalone-client-1.10.jar" *.java
if %errorlevel% neq 0 exit /b %errorlevel%
jar cvfm Client.jar manifest.txt *.class
if %errorlevel%==0 echo BUILD SUCCESSFULLY!