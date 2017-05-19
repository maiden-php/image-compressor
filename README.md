# Image Compressor

## using tinypng implementation
1. can compress 1 image at a time
2. or recursively compress all images in a directory
3. supports jpg, png, etc...

## Setup

## API
$imageCompressor = new ImageCompressor();

$imageCompressor->compressImage('path/to/my/image.jpg')->toPath('path/to/my/image.jpg')

$imageCompressor->compressFolder('path/to/my/directory')->toPath('path/to/my/directory');