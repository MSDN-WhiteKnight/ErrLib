### Script: Create ErrLib release package ###
# Should be run from "scripts" directory after the solution is built in all configurations

# Ensure "obj" and "Output" directories exist
New-Item -Path "." -Name "obj" -ItemType "directory" -ErrorAction:SilentlyContinue
Remove-Item ".\obj\*" -Recurse
New-Item -Path "." -Name "obj\x86-debug" -ItemType "directory" -ErrorAction:SilentlyContinue
New-Item -Path "." -Name "obj\x86-release" -ItemType "directory" -ErrorAction:SilentlyContinue
New-Item -Path "." -Name "obj\x64-debug" -ItemType "directory" -ErrorAction:SilentlyContinue
New-Item -Path "." -Name "obj\x64-release" -ItemType "directory" -ErrorAction:SilentlyContinue

New-Item -Path "." -Name "Output" -ItemType "directory" -ErrorAction:SilentlyContinue
Remove-Item ".\Output\*"

# Copy binaries to "obj" directory
echo ""
echo "Copying files..."

$fileList = "ErrLib.dll", "ErrLib.lib"

# x86 Debug
$inputDir = "..\Debug\"
$outputDir = ".\obj\x86-debug\"

foreach ($file in $fileList)
{
    Copy-Item ($inputDir+$file) -Destination $outputDir
}

# x86 Release
$inputDir = "..\Release\"
$outputDir = ".\obj\x86-release\"

foreach ($file in $fileList)
{
    Copy-Item ($inputDir+$file) -Destination $outputDir
}

# x64 Debug
$inputDir = "..\x64\Debug\"
$outputDir = ".\obj\x64-debug\"

foreach ($file in $fileList)
{
    Copy-Item ($inputDir+$file) -Destination $outputDir
}

# x64 Release
$inputDir = "..\x64\Release\"
$outputDir = ".\obj\x64-release\"

foreach ($file in $fileList)
{
    Copy-Item ($inputDir+$file) -Destination $outputDir
}

# Headers
$fileList = "ErrLib.h", "ErrLib_CPP.h", "ReadMe.txt"
$inputDir = "..\ErrLib\"
$outputDir = ".\obj\"

foreach ($file in $fileList)
{
    Copy-Item ($inputDir+$file) -Destination $outputDir
}

echo "Creating package..."

# Create empty ZIP archive
Copy-Item ".\empty.zip" -Destination ".\Output\"
Rename-Item -Path ".\Output\empty.zip" -NewName "ErrLib-bin.zip"

# Add files into ZIP archive
$filePath = [System.IO.Path]::GetFullPath(".\Output\ErrLib-bin.zip")
$shell = New-Object -ComObject Shell.Application
$zipFile = $shell.NameSpace($filePath)
$dir = $shell.NameSpace([System.IO.Path]::GetFullPath(".\obj"))
$zipFile.CopyHere($dir.Items())

echo "Finished"
[void][System.Console]::ReadKey($true)
