#!/usr/bin/env runhaskell
module Main where

import Development.Shake
import Development.Shake.AVR
import Development.Shake.FilePath

device  = "attiny13"
clock   = round 9.6e6

avrdudeFlags    = ["-b", "115200", "-c", "dragon_isp", "-p", device]
usbPort         = "usb"

cFlags =     ["-Wall", "-Os",
    "-DF_CPU=" ++ show clock ++ "UL",
    "-mmcu=" ++ device,
    "-Iinclude",
    "-std=c99"]

main = shakeArgs shakeOptions $ do
    want ["flicker.hex"]
    
    phony "clean" $ do
        removeFilesAfter "." ["*.o", "*.su", "*.elf", "*.hex"]
    
    phony "flash" $ do
        need ["flicker.hex"]
        avrdude avrdudeFlags "flicker.hex" usbPort
    
    "flicker.elf" *> \out -> do
        srcs <- getDirectoryFiles "." ["*.c"]
        let objs = [src `replaceExtension` "o" | src <- srcs]
        avr_ld' "avr-gcc" cFlags objs out
    
    "*.hex" *> \out -> do
        let elf = out `replaceExtension` "elf"
        avr_objcopy "ihex" ["-j", ".text", "-j", ".data"] elf out
    
    ["*.o", "*.su"] *>> \[oFile, suFile] -> do
        let src = oFile `replaceExtension` "c"
        avr_gcc (cFlags ++ ["-fstack-usage"]) src oFile
