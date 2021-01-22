#!/bin/bash
# This utility packs the Windows build of SurgeScript
# Copyright 2016-2021 Alexandre Martins

UNIX2DOS="todos" #"unix2dos"
SOURCE_FOLDER="../../.."
BUILD_FOLDER="$SOURCE_FOLDER/build"
OUTPUT_FOLDER="/tmp"

# File surgescript.exe must be present before packaging
if [ ! -f "$BUILD_FOLDER/surgescript.exe" ]; then
    echo "File surgescript.exe not found"
    exit 1
fi

# Extract the SurgeScript version
VERSION=$(wine "$BUILD_FOLDER/surgescript.exe" -v | tr -cd [:digit:][:punct:])
PACKAGE="surgescript-$VERSION-win"
echo "Found SurgeScript version $VERSION."

# Create a temporary folder for packaging
TMP_FOLDER="/tmp/$PACKAGE"
rm -rf "$TMP_FOLDER" 2>/dev/null
mkdir -p "$TMP_FOLDER"

# Write a README for Windows
cat > "$TMP_FOLDER/README-Windows.txt" << EOF
--------------------------------------------------
SurgeScript
A scripting language for games
Copyright (C) 2016-$(date +%Y)  Alexandre Martins
--------------------------------------------------
This is the Windows build of SurgeScript $VERSION.

To test SurgeScript, run the surgescript executable via the Command Prompt.
Pass the scripts you want to test via the command line, as in the examples:

  ** See the available options: **
  C:\path\to\surgescript> surgescript

  ** Run a test script: **
  C:\path\to\surgescript> surgescript examples\hello.ss

  ** Run another test script: **
  C:\path\to\surgescript> surgescript examples\count_to_10.ss

There are many example scripts to try out. Check the examples folder for
more information.

Visit the SurgeScript website at: https://github.com/alemart/surgescript/
EOF

# Copy files
for file in surgescript.exe libsurgescript-static.a libsurgescript.dll.a libsurgescript.dll surgescript.pc surgescript-static.pc; do
    echo "Copying $file..."
    cp "$BUILD_FOLDER/$file" "$TMP_FOLDER"
done

for file in LICENSE README.md CHANGES.md CMakeLists.txt mkdocs.yml; do
    echo "Copying $file..."
    cp "$SOURCE_FOLDER/$file" "$TMP_FOLDER"
done

for folder in examples src cmake; do
    echo "Copying $folder/ ..."
    cp -r "$SOURCE_FOLDER/$folder" "$TMP_FOLDER"
done

mv "$TMP_FOLDER/LICENSE" "$TMP_FOLDER/LICENSE.txt"

# Converting newlines of all text files
for f in `find "$TMP_FOLDER" -type f -exec grep -Iq . {} \; -print`; do
    ${UNIX2DOS} $f
done

# Generate the docs
pushd "$SOURCE_FOLDER"
mkdocs build
popd

# Copying the docs
for folder in docs docs_html; do
    echo "Copying $folder/ ..."
    cp -r "$SOURCE_FOLDER/$folder" "$TMP_FOLDER"
done

# Create the .zip package
pushd "$TMP_FOLDER"
echo "Packaging..."
zip -r "$OUTPUT_FOLDER/$PACKAGE.zip" ./*
echo "Packaged to $OUTPUT_FOLDER/$PACKAGE.zip"
popd
#rm -rf "${TMP_FOLDER}"