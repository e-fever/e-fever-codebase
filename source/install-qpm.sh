#!/bin/sh

(cd Flux-Application-Minimal; qpm install)
(cd Flux-Application-Full/app/MYPROJECT; qpm install)
(cd Flux-Application-Full/tests/unittests; qpm install)
