# Graph-Based Map Generation & Road Segmentation Pipeline

A complete end-to-end system that transforms fragmented satellite imagery into a stitched map, extracts road networks using deep learning, and enables path-level analysis.


## Overview

This project combines classical computer vision and deep learning to solve a real-world mapping problem:

> Input: Unordered satellite image fragments
> Output: A unified map with extracted road networks and navigable paths

###  Core Components

*  Graph-based Image Stitching
*  U-Net Road Segmentation
* Post-processing & Refinement
*  Path Visualization


## System Pipeline

```text
Image Fragments
      |
Feature Extraction (SIFT)
      |
Feature Matching
      |
Graph Construction
      |
MST Optimization
      |
Global Homography Alignment
      |
Image Warping + Blending
      |
Orthorectified Map
      |
U-Net Segmentation
      |
Post-processing (Noise removal, morphology)
      |
Final Road Map + Path Visualization

```


## How to Run
1. Load your fragments in 2nd code cell of Img_stitching.ipynb ( add dataset path to load_images function)
2. Stitched Rectified image will be saved in "output/Topdown_Map.png"
3. Run the predict_single_image function in Road_Segmentation.ipynb
4. Extracted road mask and overlay will be saved in "output/Road_mask.png" & "output/Road_overlay.png"
5. Fianlly run the find_path function in Shortest_Path.ipynb
6. Get the final path from "output/shortest_path_final.jpg"


##  Part 1: Graph-Based Image Stitching

### Methodology

* Extract SIFT features from each image
* Match features using BFMatcher + ratio test
* Construct a graph of image relationships
* Use Maximum Spanning Tree (MST) to select strongest connections
* Propagate transformations using global homographies
* Warp all images into a common coordinate frame
* Blend overlapping regions using feather blending

### Why Graph + MST?

* Avoids sequential drift
* Ensures stable global alignment
* Scales to multiple images

---

## Part 2: Road Segmentation (U-Net)

### Model

* Encoder–decoder architecture
* Skip connections preserve spatial details
* Output: pixel-wise binary mask

### Training Details

* Dataset: DeepGlobe Road Extraction
* Input size: 128 * 128
* Loss: BCEWithLogits + Dice Loss
* Optimizer: Adam (lr = 1e-4)
* Augmentation: flips

###  Why U-Net?

* Captures both context (what is road) and **location (where is road)
* Handles thin, continuous structures effectively

### Post-Processing

To improve raw predictions:

* Remove small noisy components
* Morphological closing (connect broken roads)
* Skeletonization (thin road centerlines)

> This significantly improves visual quality and structural consistency.

## Path Visualization

* Extract road mask
* Overlay on stitched map
* Draw path using connected coordinates


## Results

### Strengths

* Detects thin and curved roads
* Works across different terrains
* Good structural continuity
* End-to-end pipeline integration

### Limitations

* False positives in textured regions (fields, buildings)
* Some fragmented predictions in complex scenes
* Limited semantic understanding (texture confusion)



## Project Structure

```text
.
|- stitching
|   |_ feature_extraction
|   |_ matching.
|   |_ graph_mst
|   |_ homography
|   |_ blending
|   |_ rectification
|
|_ segmentation
|   |_ dataset
|   |_ model_unet
|   |_ train
|   |_ inference
|   |_ postprocess
|_ outputs
|   |_ TopDown_Map.png
|   |_ road_mask.png
|   |_ overlay.png
|   |_ path.png
|
|_ README.md
```


## Future Improvements

### 1. Bundle Adjustment

* Global optimization of all transformations
* Reduce accumulated alignment error

### 2. Advanced Feature Matching

* SuperPoint + SuperGlue
* Better performance in low-texture regions

### 3. Seam Optimization

* Graph-cut blending
* Multi-band blending

### 4. Improved Segmentation

* Attention U-Net / DeepLab
* Better semantic understanding

### 5. Multi-Scale Training

* Improve detection of very thin roads

### 6. Real-World Deployment

* Integration with GIS systems
* Multi-source imagery (drone + satellite)

---

## Conclusion

This project demonstrates a complete mapping pipeline, combining classical computer vision with deep learning to produce structured, meaningful outputs from raw satellite data.

---


