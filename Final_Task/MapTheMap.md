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

RANSAC - pick some matches - compute homgraphy - check all matches : yes - inlier and no - outliers





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

\-----------------------------------------------------------------------------------

Alpha feathering: In the overlap zone, linearly blend pixel values based on distance from each image's center. Simple but good enough for most cases.



Laplacian pyramid blending: Blend low frequencies gradually, high frequencies sharply. Produces seamless results even with exposure differences.



Image pyramids (Gaussian and Laplacian) — cv2.pyrDown, cv2.pyrUp

Distance transform for computing weight maps — cv2.distanceTransform

Multi-band blending algorithm



step 5 - Top-Down 2D map

\---------------------------------------------------------------------------------------

1. First see if rectification is needed , cz my dataset already has rectified fragments -> 2D map



2\. Detect the corners /boundary

* Convert image -> mask of “non-black” pixels
* Find largest connected region (your map)
* Compute convex hull -> smooth outer boundary  ( sA, aM.A)
* Approximate that shape to a polygon (4 points) ( ApproxPolyDP - Douglas-Peucker Algo)

epsilon - max allowed distance between original and simplified curve

small epsilon → keep details -> many points

large epsilon → remove details -> fewer points

for any segment in approximation - all points must lie within distance <= epsilon from that segment

instead of using fixed pixels we use percentage of shape size , so it becomes scale invariant -> small obj - small epsilon , large obj - large epsilon

&#x20;beyond 0.1 shape collapses much to triangle / line

DP A-

* Take first and last point - draw a line
* find point farthest from this line
* if distance> epsilon - keep that point or else discard intermediate values
* recursivey apply on segments



Alternatives :

* PCA -based  oriented bounding box ( principal component analysis)

Steps:

Take all contour points

Compute covariance matrix

Eigenvectors → principal directions

Project points → get min/max → rectangle

Pros

Stable orientation (noise-resistant)

No iterative search

Cons

Still approximates shape as rectangle

Needs implementation (OpenCV doesn’t give full box directly)



* Hough line detection( geometry - driven)

detect long straight edges , then intersect them

Pros

Works well when edges are visible (roads, borders)

Geometry-based, not contour-based

Cons

Sensitive to noise

Needs grouping logic

Use when: map edges are visually strong lines.



* RANSAC quad fitting

Randomly sample 4 points and evaluate if they form a good rectangle.

steps:

pick 4 candidate points

compute homography

measure how well points fit edges

repeat (RANSAC)

Pros

Robust to noise/outliers

Doesn’t rely on perfect contour

Cons

More complex

slower than deterministic methods



* Distance - transform/extreme points

works directly on mask and find distant points

very simple can be unstable if shape not well distributed



\*\*\*\*\*\*\*\*\*\*\*\*try only minRectVersion\*\*\*\*\*\*\*\*\*\*\*'

If this also fails : fallback -> minimum area rectangle

* rect with min area that encloses all point



Aspect ratio - we cannot gave fixed values to final rectangle

Output: 4 corner points \& aspect ratio of region4



3\. Apply homography

I have 4 source points (skewed quadrilateral)-> map them to a perfect rectangle

\-can use a 3×3 transformation matrix (homography)



* Compute width and height from points
* Create final rectangle
* Compute transformation matrix
* Warp image



Step 6 — Road segmentation

\-------------------------------------------------------------------------------------

Consistent grayish/asphalt color (but can vary)

Long, thin, connected regions

Relatively smooth texture compared to surroundings



Classical pipeline:

* Preprocessing - resize to fixed scale , CLAHE on L(LAB), Gaussian denoise

( Why normalise lightning across fragments )

* Convert to LAB or HSV color space (roads easier to isolate there) \& Apply color threshold for road-colored regions             -A
* LBP variance map - roads smooth and low var     - B
* Use Canny edge detection to find road boundaries   -C
* pixel = road if (W.A + W.B + W.C) > threshold
* Morphological closing to fill road interiors, opening to remove noise
* Filter contours by area, aspect ratio, and elongatedness to keep only road-like shapes
* Shadow and building removal ( Dark region mask subtract from road mask , buildings - square)
* Centerline - refinement : Shang-Suen skeleton -> prune spurs -> smooth centerline

The result is a binary road mask



Research on :

* Morphological operations: erosion, dilation, opening, closing



* Canny edge detector: how it uses gradients + non-max suppression + hysteresis
1. Noise reduction : using a 5\*5 Gaussian filter

2\. Finding Intensity gradient of the image : Smoothened image is then filtered with a Sobel kernel in both horizontal and vertical direction to get first derivative in horizontal direction (Gx) and vertical direction (Gy)

&#x20;      Edge\_Gradient (G) = sqt(Gx^2 + Gy^2)Angle(thita) = tan-1(Gy/Gx)

Gradient direction is always perpendicular to edges. It is rounded to one of four angles representing vertical, horizontal and two diagonal directions.

3\. Non-maximum Suppression  :



* Watershed algorithm (useful if roads need to be separated from similar-colored surfaces)
* Contour analysis: cv2.findContours, bounding rects, aspect ratios



Pure classical road segmentation is a bit off like results will depend heavily on how the images look.

* iteration on the color thresholds.



U-NET Road Segmentation

\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

* upsampling operators instead of pooling operations
* the network employs a symmetric u-shaped structure
* large no of feature channels in the upsampling part- extract and utilize rich contextual information, leading to more accurate segmentations.
* Encoder : captures high level features from input image 

&#x20; -  series of conv -> ReLu actvn fn -> padding(maintain spatial dimensions)

&#x20; - Conv - filters to extract features 

&#x20; - MaxPooling - performed to downsampple the feature map and reduce spatial dimensions.

* Bridge - connects encoder and decoder ( additional conv with relu act)

&#x20;   - preserving spatial information by concatenating the feature maps from the encoder to the corresponding decoder layers.

* Decoder - generates the final segmentation map using the concatenated feature maps from the bridge

&#x20;  - upsampling

&#x20;  -  ends with a convolutional layer with a sigmoid activation function to produce the final segmentation map.

* uses skip connection 
* multiple feature detectors to create multiple feature maps
* stride is the number of steps that the feature detector takes while navigating over the input image



Deconvolution :

\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_

* upsampling : insert empty rows and columns between existing elements.
* convolution : a conv layer applied to unsampled map ( applies a set of learnable filters to feature map - extract features ) 
* stride : using s >1 during conv layer  - step size while sliding the filters over the input - >1 increases the spatial dimensions of output feature map 
* &#x20;padding : additional rows and columns of zeros around the input feature map, preserving its size during the convolution operation. 
* activation fn 







Implementation : 

1. two consecutive conv layer in each block 
2. left side - contraction - encoder 
3. right side - expansion - decoder - applies transposed convolutions along with the regular convolutions 
4. In the Encoder, the input image gradually reduces in size while the depth (number of channels) increases. For example, the image may go from 572x572x3 to 284x284x128
5. Encoder learns the "WHAT" information in the image, but it loses the "WHERE" information
6. the size of the image gradually increases, while the depth decreases. For instance, the image may go from 8x8x256 to 128x128x1.
7. recovers the "WHERE" information by gradually applying up-sampling to obtain precise localization.
8. o obtain precise localization.
9. To achieve more accurate localization, skip connections are employed. These connections involve concatenating the output of transposed convolutional layers with the corresponding feature maps from the Encoder:
- u6 = u6 + c4
- u7 = u7 + c3
- u8 = u8 + c2
- u9 = u9 + c1

&#x20;  After concatenation, two consecutive regular convolutions are applied to refine the output.











Step 6 \& 7 — Skeleton, graph, and pathfinding (Bonus)

\-----------------------------------------------------------------------------





Skeletonization: Thin the binary road mask to a 1-pixel-wide centerline using morphological thinning or skimage.morphology.skeletonize. This gives you the "spine" of every road.



Graph extraction: Traverse the skeleton. Pixels with exactly 2 neighbors are just road segments. Pixels with 3+ neighbors are intersections (graph nodes). 

Pixels with 1 neighbor are dead ends. 

Build a networkx graph from this.



Shortest path: Dijkstra or A\* with Euclidean heuristic. Overlay this path on the stitched map with a colored line.





Morphological skeletonization (Zhang-Suen thinning algorithm)

Graph representation using networkx or just adjacency dicts

Dijkstra's algorithm and A\* 





CHANGES IMPLEMENTED



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





dfs 







