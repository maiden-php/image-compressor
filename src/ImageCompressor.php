<?php

namespace API\Components\Utilities;

/**
 * TinyPNG Implementation of compressing an image
 *
 * Class ImageCompressor
 * @package API\Components\Utilities
 */
class ImageCompressor implements ImageCompressionInterface
{
    /**
     * Compresses an image from a $file_path and moves it into the $file_path directory
     *
     * @param $file_path
     * @return bool|string
     */
    public function compressImage($file_path) {
        \Tinify\setKey("mdFC3gspl3r1_EBaKU5GevJGNs9Nyw2m");

        try {
            $source = \Tinify\fromFile($file_path);
            $source->toFile($file_path);
        }
        catch (\Exception $e) { // if it is NOT an image then we don't do anything
            return $e->getMessage();
        }

        return 'success';
    }
}
