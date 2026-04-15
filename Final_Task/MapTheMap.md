1\. Final Goal

Input: Random unordered map image pieces

Output:

Grouped regions

Stitched full maps

Roads extracted



2\. FULL PIPELINE



Input Images

&#x20;     |

Feature detect

&#x20;     |

Feature Matching (pair wise)

&#x20;     |

Similarity Matrix /Homography

&#x20;     |

Build graph

&#x20;     |

Ordered Stitching 

&#x20;     |

Final Map ( top down 2D )

&#x20;     |

Road Extraction

&#x20;     |

Graph Builder

&#x20;     |

Path Finder





Final Structure : -

|---	fragments.py

|---	stitching.py

|---	segmentation.py

|---	pathfinding.py

|---	main.py





ORB/SIFT

BFMatcher

Homography

WarpPerspective

Canny Edge

Morphology

Basic Graph





Library

opencv-python : Everything — features, matching, warping, morphology

numpy : Array math throughout

scikit-image : Skeletonization (skimage.morphology.skeletonize)

network : Graph building and Dijkstra

matplotlib : Visualization



Step 1 - Feature extraction

\--------------------------------------------------------------

Every image fragment - detect distinctive keypoints and compute a descriptor (a compact vector).

descriptors : let you recognize the same physical location across two different image pieces.

\- ORB (Oriented FAST and Rotated BRIEF)

How FAST corner detection works (intensity circle comparison)

What a binary descriptor is and why Hamming distance is used to compare them

Scale-space and image pyramids (why we need multi-scale detection)



cv2.ORB\_create()



\*\* some key functions : \*\*

1\. cv2.ORB\_create(

&#x09;nfeatures =

&#x09;scaleFactor=

&#x09;nlevels =

&#x09;edgeThreshold=

&#x09;patchsize =

&#x20;       )

&#x20;  or 



sift = cv2.SIFT\_create()



2\. orb.detectAndComputr(gray\_img, None)   # detects keypoint and calculate descriptor

3\. cv2.cvtColor(img, cv2.COLOR\_BGR2GRAY)    #to gray

4. cv2.eualizeHist(gray\_img)   #used to improve image contrast by redistributing pixel intensity values across the entire range , effective in images where the foreground and background have similar brightness



&#x20;      \*\* working of histogram equalisation \*\*

&#x20;  - create a histogram of the image which shows how often each pixel value occurs

&#x20;  - calculate the cumulative distribution of pixel intensities (CDF) :how pixel intensities are spread

&#x20;  - normalize the CDF so that the pixel values can be mapped to a new range (usually 0-255) : ensure even spread

&#x20;  - Transformation : normalized CDF to map the original pixel values to new ones ( img with improved contrast)

&#x20;    uses : medical imaging , satellite \& aerial imaging , surveillance and security (vdo footage)





Step 2 - Image matching and grouping

\---------------------------------------------------------------------

Take descriptors from every pair of images and try to match them. If two images share many strong matches, they likely overlap and belong to the same map.



* BFMatcher with Hamming distance (for ORB binary descriptors).
* Apply Lowe's ratio test to filter bad matches.
* Compute a homography between matched pairs using RANSAC (if RANSAC finds a valid homography with enough inliers, the pair is genuine overlapping fragments.)
* overlap graph: nodes are images, edges - overlap.
* Find connected components of this graph using Union-Find or BFS/DFS. (Each connected component is one map group)



&#x20;- Lowe's ratio test (why the ratio of nearest to second-nearest neighbor matters)

&#x20;- RANSAC algorithm — how it robustly fits a model with outliers

&#x20;- Homography matrix:(ek matrix jo batata hai ki img A ka point img B mein kaha hai) what it is, what 8 DOF mean geometrically

&#x20;- Connected components on graphs



\*\* some key functions : \*\*

1. bf = cv2.BFMatcher(cv2.NORM\_HAMMING, crossCheck=False) #query Brute-Force M , When crossCheck=True, a match between two descriptors is only returned if they are mutually each other's best match.

&#x20;  cv2.NORM\_HAMMING : hamming distance for comparing binary feature descriptors

2\. bf.knnMatch(desc1, desc2, k=2)   #k represents the number of closest neighbors to return for each query descriptor and train descriptors

3\.  H, mask = cv2.findHomography(src\_pts, dst\_pts, cv2.RANSAC, 5.0)

(open cv internally considers desc1 and desc2 as

4\. inliers - correct matches that agree with geometry

RANSAC - pick some matches - compute homgraphy - chreck all matches : yes - inlier and no - outliers





Step 3 — Homography estimation and warping to top-down view

\-----------------------------------------------------------------------------



you have images belong together, stitch them by computing the homography between each pair and warping images onto a shared canvas.

* top-down conversion: Aerial/map images are typically already close to top-down, but if there's perspective tilt, you can correct it by computing a homography that maps the image plane to a flat ground plane.

If you have a reference image that looks properly top-down, warp everything to match it.

For the reference frame: pick one image as the "anchor" and compute homographies from every other image relative to it. For longer chains (A→B, B→C), compose homographies: H\_AC = H\_BC @ H\_AB.



* The 3×3 homography matrix and how cv2.warpPerspective uses it
* How to compute the output canvas size when warping (find where image corners map to)
* Bundle adjustment (if you want to go deeper — minimizes accumulated drift across many images)



To stitch two images, you need

* Find common points (features)
* Match them
* Compute transformation (Homography)
* Warp one image onto another



1. Sequencial Stitching - one by one

but each steps adds error - error accumulation.



2\. Graph Based Stitching - Build a network of all images

* Treat images as nodes
* Match ALL pairs , For every pair: find matches \& compute homography
* Build graph ( Edges = valid matches )

Image1 - Image2

&#x20;  |         |

Image3 - Image4

* Choose reference image
* Align everything to reference ( Using BFS )
* Warp all images to same plane

No drift , Works for unordered images, More accurate

BUT more computation and complex



3\. Optimization-based - Used in real systems:

* Bundle Adjustment

adjusts ALL images simultaneously and minimizes global error



Used in: Google Street View, SLAM, drones





My approach :

* Use sequential to find order

&#x20;      - compute matches between all pairs

&#x20;      - sort by match count

&#x20;      - build chain

* Use graph method for alignment

&#x20;      - build graph from ordered images

&#x20;      -compute homographies

&#x20;      -align globally

* Blend

&#x20;



Warping = bending / transforming an image so it aligns with another



BFS = breadth first search - starts from a source node and explores the graph level by level ( first adjacent nodes to main nodes and then adjacent to adjacent node )





Step 4 — Blending

\----------------------------------------------------------------------------------

seams -

blending - 



What you need to do: Where images overlap after warping, you'll see hard seams if you just paste one on top of the other. Blending smooths this out.

Two approaches:



Alpha feathering: In the overlap zone, linearly blend pixel values based on distance from each image's center. Simple but good enough for most cases.

Laplacian pyramid blending: Blend low frequencies gradually, high frequencies sharply. Produces seamless results even with exposure differences.



What to study:



Image pyramids (Gaussian and Laplacian) — cv2.pyrDown, cv2.pyrUp

Distance transform for computing weight maps — cv2.distanceTransform

Multi-band blending algorithm





Step 5 — Road segmentation

\-------------------------------------------------------------------------------------

Consistent grayish/asphalt color (but can vary)

Long, thin, connected regions

Relatively smooth texture compared to surroundings



Classical pipeline:

* Convert to LAB or HSV color space (roads easier to isolate there)
* Apply color thresholding for road-colored regions
* Use Canny edge detection to find road boundaries
* Morphological closing to fill road interiors, opening to remove noise
* Filter contours by area, aspect ratio, and elongatedness to keep only road-like shapes

The result is a binary road mask



Research on :

* Morphological operations: erosion, dilation, opening, closing 
* Canny edge detector: how it uses gradients + non-max suppression + hysteresis
* Watershed algorithm (useful if roads need to be separated from similar-colored surfaces)
* Contour analysis: cv2.findContours, bounding rects, aspect ratios



i feel pure classical road segmentation is a bit off like results will depend heavily on how the images look.

* iteration on the color thresholds.





Step 6 \& 7 — Skeleton, graph, and pathfinding (Bonus)

\-----------------------------------------------------------------------------



Skeletonization: Thin the binary road mask to a 1-pixel-wide centerline using morphological thinning or skimage.morphology.skeletonize. This gives you the "spine" of every road.

Graph extraction: Traverse the skeleton. Pixels with exactly 2 neighbors are just road segments. Pixels with 3+ neighbors are intersections (graph nodes). Pixels with 1 neighbor are dead ends. Build a networkx graph from this.

Shortest path: Once you have the graph, Dijkstra (or A\* with Euclidean heuristic) gives you the shortest road path between any two points. Overlay this path on the stitched map with a colored line.

What to study:



Morphological skeletonization (Zhang-Suen thinning algorithm)

Graph representation using networkx or just adjacency dicts

Dijkstra's algorithm and A\* — you already implemented A\* in Task 5, so this is a direct reuse



cganges that can be done : Image Stitching )

Basic: ORB → match → homography → warp

instead of random stitching:

Stitch pairwise in order

Maintain canvas growth strategy

EVEN BETTER:

Matches visualization

Intermediate stitching steps

Dynamic canvas size calculation

Most : Hardcode canvas

try : Compute transformed corners \& Expand canvas dynamically



&#x20;Road Segmentation

Basic:

Canny edge

Threshold

Try using combo:

Color filtering (roads are grayish)

Edge detection

Morphological closing







SURF (Speeded-Up Robust Features) is a high-performance computer vision algorithm used to detect, describe, and match local features (interest points) in images. Developed by Bay et al. in 2006, it was designed as a faster, more efficient alternative to the SIFT (Scale-Invariant Feature Transform) algorithm while maintaining similar robustness to image transformations like rotation, scaling, and brightness changes.

SURF is widely used in real-time applications such as object recognition, image stitching, 3D reconstruction, and tracking.



Key Components of the SURF Algorithm

SURF achieves its speed and robustness through three main stages:

1\. Interest Point Detection (Fast-Hessian Detector)

Unlike SIFT, which uses Difference of Gaussians (DoG), SURF uses a Hessian matrix-based detector to find keypoints.



Hessian Matrix: It calculates the determinant of the Hessian matrix, which measures local intensity changes to detect blob-like structures (corners or blobs).

Box Filters \& Integral Images: To make this process extremely fast, SURF approximates Gaussian second-order derivatives using simple box filters. The convolution of these box filters with the image can be calculated in parallel and in constant time, regardless of the filter size, using Integral Images (summed-area tables).

Scale Space: Instead of reducing the image size (image pyramid) like SIFT, SURF increases the size of the box filters (e.g.,) to detect features at different scales.

Non-Maximum Suppression: Potential keypoints are found by looking for local maxima of the Hessian determinant in both the image and scale space.



2\. Orientation Assignment

To ensure rotation invariance, SURF assigns a reproducible orientation to each keypoint.

Haar Wavelets: SURF computes Haar wavelet responses in

&#x20;and

&#x20;directions around the keypoint in a circular neighborhood of radius

&#x20;(

&#x20;is the scale of the keypoint).

Dominant Orientation: The responses are weighted with a Gaussian, and the sum of responses is calculated using a sliding window of



&#x20;(60 degrees). The longest, most dominant orientation vector is then assigned to the keypoint.

U-SURF (Upright SURF): If orientation assignment is skipped for speed (useful if images are known to be upright), it is called U-SURF.



3\. Feature Description

Once a keypoint is located and oriented, a descriptor is created to represent it.

Medium

Medium

Region Selection: A square region of size



&#x20;is constructed around the keypoint, oriented along the dominant orientation.

Sub-regions: The region is divided into



&#x20;sub-regions.

Wavelet Features: In each sub-region, Haar wavelet responses (





) are computed at



&#x20;regularly spaced sample points.

64-Dimensional Vector: The responses are summed to form a 4D vector





















&#x20;for each sub-region. With



&#x20;sub-regions, this results in a final 64-dimensional feature descriptor.

Robustness: The use of sums of wavelet responses provides robustness against noise, illumination changes, and geometric distortions







CHANGES IMPLEMENTING 

\------------------------------------------------------------------



1. Same pre-processing - gray , equalize hist
2. Feature extraction - use sift instead of orb
3. Matching pairs - Ratio test threshold to 0.75 (0.9) and removed fallback strategy
4. Build Graph ( before homography ) - show graph 
5. group find - unnecessary codes - remove 
6. Create components for group         (\*\*\*Community \*\*\*)
7. see group wise visualization \*\*
8. Use mst to find order  ( earlier adjacent nodes and neighbors)
9. homography ( no need to return mask also )      {inliers = int(mask.sum())}
10. Global\_Homography : 
11. (Stitch pair : 2 image at a time)( Stitch whole component using order and wrap2img)
12. warp all images at a time ( into same space )
13. Find seams - cv2.detail\_GraphCutSeamFinder("COST\_COLOR")
14. Blending only once - final- MultiBandBlender
15. crop out black part











dfs ?!

















































