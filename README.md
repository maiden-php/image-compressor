[![Build Status](https://travis-ci.org/alexgarrett/violin.svg?branch=master)](https://travis-ci.org/alexgarrett/violin) [![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/alexgarrett/violin?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Image Compressor
An Image Compressor that will compress the size of any one image or multiple images from a directory.
Currently supports .jpg, .jpeg, .png extensions. Also supports drivers for TinyPNG, Kraken, and Compressor.io 

## Installation
Install using Composer.
```json
{
    "require": {
        "maiden/image-compressor": "1.*"
    }
}
```

## Usage

#### Setup
```php
$driver     = 'tinypng';
$api_key    = '1234567890';
$imageCompressor = new ImageCompressor($driver, $api_key);
```

#### Compressing one image at a time
```php
$imageCompressor->compressImage('path/to/my/image.jpg')->toPath('path/to/my/image.jpg')
```

#### Compressing a whole directory of images recursively
```php
$imageCompressor->compressFolder('path/to/my/directory')->toPath('path/to/my/output/directory');
```