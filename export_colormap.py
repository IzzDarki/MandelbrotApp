import matplotlib.pyplot as plt
import colorcet as cc
import numpy as np

# The 'Cyclic' group is handled differently in the C++ code (interpolation wrapping around )
cmap_groups = {
    'Perceptually Uniform': ['viridis', 'plasma', 'inferno', 'magma', 'cividis', 'cet_rainbow', 'cet_rainbow4', 'cet_kbc', 'cet_kgy', 'cet_fire'],
    'Sequential': ['Greys', 'Purples', 'Blues', 'Greens', 'Oranges', 'Reds',
                      'YlOrBr', 'YlOrRd', 'OrRd', 'PuRd', 'RdPu', 'BuPu',
                      'GnBu', 'PuBu', 'YlGnBu', 'PuBuGn', 'BuGn', 'YlGn'],
    'Sequential2': ['binary', 'gist_yarg', 'gist_gray', 'gray', 'bone',
                      'pink', 'spring', 'summer', 'autumn', 'winter', 'cool',
                      'Wistia', 'hot', 'afmhot', 'gist_heat', 'copper'],
    'Diverging': ['PiYG', 'PRGn', 'BrBG', 'PuOr', 'RdGy', 'RdBu', 'RdYlBu',
                      'RdYlGn', 'Spectral', 'coolwarm', 'bwr', 'seismic'],
    'Diverging Dark': ['berlin', 'managua', 'vanimo', 'cet_bkr', 'cet_bky'],
    'Cyclic': ['twilight', 'twilight_shifted', 'hsv', 'cet_colorwheel'],
    'Isoluminant': ['cet_isolum', 'cet_gwv'],
    'Miscellaneous': ['ocean', 'gist_earth', 'terrain',
                      'gist_stern', 'gnuplot', 'gnuplot2', 'CMRmap',
                      'cubehelix', 'brg', 'gist_rainbow', 'rainbow', 'jet',
                      'turbo', 'nipy_spectral', 'gist_ncar']
}

# 3. Write out the binary file
with open('colormaps.bin', 'wb') as f:
    for group, names in cmap_groups.items():
        for name in names:
            # Write group name (padded to 32 bytes)
            f.write(group.encode('utf-8').ljust(32, b'\0'))
            
            # Write colormap name (padded to 32 bytes)
            display_name = name # name.replace('cet_', '')
            f.write(display_name.encode('utf-8').ljust(32, b'\0'))
            
            # Extract RGB, flatten, and write as 32-bit floats
            colors = plt.get_cmap(name)(np.linspace(0, 1, 256))[:, :3]
            f.write(colors.astype(np.float32).tobytes())