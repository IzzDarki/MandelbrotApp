A simple app that lets you zoom into a visualization of the mandelbrot set.
The zoom depth is limited by the accuracy of floating point numbers.

### Screenshots
![Sceenshot 1](screenshots/Mandelbrot-Screenshot-1.png)
![Sceenshot 2](screenshots/Mandelbrot-Screenshot-2.png)
![Sceenshot 3](screenshots/Mandelbrot-Screenshot-3.png)
![Sceenshot 4](screenshots/Mandelbrot-Screenshot-4.png)
![Sceenshot 5](screenshots/Mandelbrot-Screenshot-5.png)
![Sceenshot 6](screenshots/Mandelbrot-Screenshot-6.png)
![Sceenshot 7](screenshots/Mandelbrot-Screenshot-7.png)

### Build
I am not an expert on CMake, but the following works:
1. `cd` to `build/` (and ensure it is empty)
    ```shell
    rm -rf build
    mkdir build
    cd build
    ```
2. Run `cmake` while specifying release or debug configuration (`Release` or `Debug`)
    ```shell
    cmake --DCMAKE_BUILD_TYPE=Release ..
    ```
3. Build
    ```shell
    cmake --build . -j
    ```
4. Find the compiled binary in `bin-release/` or `bin-debug/`
