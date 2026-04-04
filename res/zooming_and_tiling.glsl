
uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 center;
uniform uvec2 tileOffset; // used for tiled screenshot rendering
double windowSizeMeasure = double(min(windowSize.x, windowSize.y));

dvec2 pixelCoordToPlaneCoord(dvec2 pixelCoord) {
    return (zoomScale / windowSizeMeasure) * (pixelCoord + dvec2(tileOffset) - dvec2(windowSize) / 2.0) + center;
}
