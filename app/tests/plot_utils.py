import matplotlib.pyplot as plt
import numpy as np



def plot_images(images, row_labels, col_labels, figsize, x_label, y_label, title):
    s = images[0][0].shape

    h = s[0]
    w = s[1]

    for row in images:
        for i in row:
            if (i.shape != s):
                print("all images must have the same shape")
                return
            
    rows = images.shape[0]
    cols = images.shape[1]

    t = images[0][0].dtype

    image_grid = np.zeros((rows * h, cols * w, s[2]), dtype=t) if (len(s) == 3) else np.zeros((rows * h, cols * w), dtype=t)

    for y in range(rows):
        for x in range(cols):
            image = images[y][x]
            y_offset = y * h
            x_offset = x * w
            if (len(s) == 3):
                image_grid[y_offset:(y_offset + h), x_offset:(x_offset + w), :] = image
            else:
                image_grid[y_offset:(y_offset + h), x_offset:(x_offset + w)] = image

    if (np.issubdtype(t, np.floating)):
        mi = np.min(image_grid)
        ma = np.max(image_grid)
        image_grid = ((image_grid - mi) / (ma - mi)) * 255.0

    image_grid = image_grid.astype(np.uint8)
    
    fig, ax = plt.subplots(figsize=(figsize, figsize * int(image_grid.shape[1] / image_grid.shape[0])))
    
    cax = ax.imshow(image_grid, aspect="equal")

    ax.set_xticks(np.arange(w // 2, len(col_labels) * w, w))
    ax.set_xticklabels(col_labels)
    ax.set_yticks(np.arange(h // 2, len(row_labels) * h, h))
    ax.set_yticklabels(row_labels)
    
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)

    ax.set_title(title)
    
    plt.tight_layout()
    plt.show()

def plot_heatmap(matrix, row_labels, col_labels, meassurment_unit, x_figsize, y_figsize, x_label, y_label, title):
    fig, ax = plt.subplots(figsize=(x_figsize, y_figsize))
    cax = ax.imshow(matrix, aspect="auto")

    fig.colorbar(cax)

    ax.set_xticks(np.arange(len(col_labels)))
    ax.set_xticklabels(col_labels)
    ax.set_yticks(np.arange(len(row_labels)))
    ax.set_yticklabels(row_labels)

    plt.setp(ax.get_xticklabels(), rotation=0, ha="center")

    for i in range(matrix.shape[0]):
        for j in range(matrix.shape[1]):
            ax.text(
                j, 
                i, 
                str(round(matrix[i, j], 3)) + " " + meassurment_unit,
                ha="center", 
                va="center",
                color=("white" if matrix[i, j] < (matrix.min() + (matrix.max() - matrix.min()) / 2.0) else "black")
            )

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)

    ax.set_title(title)

    plt.tight_layout()
    plt.show()