# PunkDist - A distortion pedal plugin
[![PunkDist multiplatform build](https://github.com/gmoican/PunkDist/actions/workflows/main.yml/badge.svg)](https://github.com/gmoican/PunkDist/actions/workflows/main.yml)

![DemoImage](docs/images/demo.jpg)

## Introduction
This is a simple vst3/au distortion plugin made with [JUCE](https://juce.com/) that I built to introduce myself in the JUCE framework. The functionality is inspired by the [MiniDist by Markbass](https://www.markbass.it/product/mb-mini-dist/), although it doesn't sound the same (yet). The project was mostly used to further my understanding and knowledge of digital signal processing and digital audio effects.

## Features
- Drive gain (from 0 up to 45 dB).
- Output Gain (from -30 up to 30 dB).
- Tone 1: Selects the frequency (from 200 up to 2500 Hz) at which a small boost is applied. The boost amount also increases with the frequency, in fact, no boost is applied when the knob is at its minimum. 
- Tone 2: Dips the signal (from 0 up to -10 dB) at 800 Hz. It also applies a little bump at 80 Hz.

## TODO
- I have to do a lot of testing and measures to replicate the distortion of the MiniDist, this implies that I should redo the whole drive processor.