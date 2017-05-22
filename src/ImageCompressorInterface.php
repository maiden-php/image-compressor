<?php

namespace Maiden\Components;

interface ImageCompressionInterface {

    public function compressImage(string $file_path) : string;

}