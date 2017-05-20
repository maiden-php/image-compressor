[![Build Status](https://travis-ci.org/alexgarrett/violin.svg?branch=master)](https://travis-ci.org/alexgarrett/violin) 
[![Latest Stable Version](https://poser.pugx.org/laravel/passport/v/stable.svg)](https://packagist.org/packages/laravel/passport)
[![Latest Unstable Version](https://poser.pugx.org/laravel/passport/v/unstable.svg)](https://packagist.org/packages/laravel/passport)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

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

### Setup
```php
$driver     = 'tinypng';
$api_key    = '1234567890';
$imageCompressor = new \Maiden\Components\ImageCompressor($driver, $api_key);
```

### Compressing a single image
```php
$imageCompressor->compressImage('path/to/my/image.jpg')->toPath('path/to/my/image.jpg')
```

### Compressing a directory of images
```php
$imageCompressor->compressImagesInFolder('path/to/my/input/directory')->toPath('path/to/my/output/directory');
```

### Logging Errors
If there is an error with the image compression, then the error message will be logged 
to a specified directory, if not specified then it will log to the cwd/log