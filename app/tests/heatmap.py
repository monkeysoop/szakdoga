import matplotlib.pyplot as plt
import numpy as np

def plot_heatmap(matrix, row_labels, col_labels, meassurment_unit, x_figsize, y_figsize, x_label, y_label, title):
    fig, ax = plt.subplots(figsize=(x_figsize, y_figsize))
    cax = ax.imshow(matrix, aspect='auto')

    cbar = fig.colorbar(cax)

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