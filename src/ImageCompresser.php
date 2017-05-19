<?php

namespace API\Components\Utilities;

set_time_limit(0);
ini_set('memory_limit', '-1');

abstract class ImageCompression {

    public $var;

    abstract public function compressImage($image);

    private function helper() {

    }
}

class ImageCompresser extends ImageCompression
{
    public function compressImage($image) {
        // TODO: Implement compression() method.
    }

    public function doGiganticImageCompression() {
        // init the TinyPNG API
        \Tinify\setKey("mdFC3gspl3r1_EBaKU5GevJGNs9Nyw2m");

        // we first get everything single file.name recursively in $src_path,
        // then append them all into an array
        $src_path = getcwd() . '/sites/springfreetrampoline.com/files/';
        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($src_path));
        $files = [];
        foreach ($rii as $file) {
            if (!$file->isDir()) {
                $files[] = $file->getPathname();
            }
        }

        // creates the files/folders into a new folder and compress every single image file with the tinypng api
        // and copies any other file that is NOT an image
        foreach ($files as $file) {
            $new_file = str_replace('springfreetrampoline.com', 'springfreetrampoline.com2', $file);

            if (! folder_exist(dirname($new_file))) {
                echo 'directory does not exist, so we create a new one here. ', dirname($new_file), '<br>';
                mkdir(dirname($new_file), 0777, TRUE);
            }

            if (folder_exist(dirname($new_file))) {
                echo 'directory exists, so we create the file in that directory. ', $new_file, '<br>';

                // have to check whether it's an image file or not, only then do we use tinypng to convert
                try {
                    $source = \Tinify\fromFile($file);
                    $source->toFile($new_file);
                }
                catch (\Exception $e) { // if it is NOT an image then we don't do anything
                    echo "---------- The error message is: " . $new_file . ' is not an image file, so we are copying it ' . '<br>';
                    copy($file, $new_file);
                }
            }
        }
        return;
    }
}

(new ImageCompresser())->compressImage('path/to/image');


function folder_exist($folder) {
    // Get canonicalized absolute pathname
    $path = realpath($folder);

    // If it exist, check if it's a directory
    return ($path !== FALSE AND is_dir($path)) ? $path : FALSE;
}