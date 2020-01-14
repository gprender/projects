# A couple of interesting problems I came across during my co-op at GeoBC.
# The first function is a fun case of finding the intersection between two boxes,
# and the second is a quick vectorized implementation of principal component analysis.
#
# Written by Graeme Prendergast, Fall 2019

import laspy
import numpy as np


def crop_to_intersection(fp1, fp2):
    """
    Crop a pair of lidar files to their largest region of intersection.

    Args:
        fp1 : filepath1, absolute path to the first lidar file
        fp2 : filepath2, absolute path to the second lidar file
    Returns:
        (points1_cropped, points2_cropped) : a 2-tuple of point
            coordinates from fp1 and fp2 respectively, which fall
            inside of the two infiles' region of intersection
    """
    # Create two read-only laspy file objects
    infile1 = laspy.file.File(fp1, mode='r')
    infile2 = laspy.file.File(fp2, mode='r')

    # Read the raw [x,y,z] coordinates from each lidar file
    points1 = np.vstack([infile1.x, infile1.y, infile1.z]).transpose()
    points2 = np.vstack([infile2.x, infile2.y, infile2.z]).transpose()

    # Create a bounding box object from the header of each lidar file
    a = BoundingBox(infile1.header.min[0], infile1.header.max[0],
                    infile1.header.min[1], infile1.header.max[1])

    b = BoundingBox(infile2.header.min[0], infile2.header.max[0],
                    infile2.header.min[1], infile2.header.max[1])

    # Calculate bounding box of the intersection
    ab = BoundingBox(max(a.xmin, b.xmin), min(a.xmax, b.xmax),
                     max(a.ymin, b.ymin), min(a.ymax, b.ymax))

    # If the two lidar swaths intersect:
    if ((ab.xmax - ab.xmin > 0) and (ab.ymax - ab.ymin > 0)):
        # Incrementally build up a boolean "filter array" to find which
        # points in points1 fall inside of the intersection ab
        p1_xfilter = np.logical_and((points1[:,0] >= ab.xmin),
                                    (points1[:,0] <= ab.xmax))
        p1_yfilter = np.logical_and((points1[:,1] >= ab.ymin),
                                    (points1[:,1] <= ab.ymax))
        p1_xyfilter = np.logical_and((p1_xfilter), (p1_yfilter))

        # Likewise for points2
        p2_xfilter = np.logical_and((points2[:,0] >= ab.xmin),
                                    (points2[:,0] <= ab.xmax))
        p2_yfilter = np.logical_and((points2[:,1] >= ab.ymin),
                                    (points2[:,1] <= ab.ymax))
        p2_xyfilter = np.logical_and((p2_xfilter), (p2_yfilter))
    else:
        raise ValueError("The given lidar files do not overlap.")

    return points1[p1_xyfilter], points2[p2_xyfilter]


def pca(neighborhood):
    """
    Perform principal component analysis on a given point cloud.

    This function is obviously in the context of lidar data, but is
    generalized to work with any n-dimensional point cloud data.

    Args:
        neighborhood : an (M x N) numpy array of n-dimensional points
    Returns:
        A tuple of the form ((eigenvalue, eigenvector[]), ...) given 
		by the principal components of the input neighborhood. For an 
		input array of shape (M x N), this tuple will have N entries.
    """
	# Define the dimensions of the array
    m, n = neighborhood.shape
	
    # Center the neighborhood of points about the origin
    mean_vector = np.mean(neighborhood, axis=0)
    centered_nhood = neighborhood - mean_vector

    # Perform the principal component analysis
    neighborhood_cov = np.cov(np.transpose(centered_nhood))
    eigenvalues, eigenvectors = np.linalg.eig(neighborhood_cov)

    return ((eigenvalues[i], eigenvectors[:,i]) for i in range(0, n))


class BoundingBox:
    """Describes a geometric rectangle by its bounding coordinates"""
    def __init__(self, xmin, xmax, ymin, ymax):
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax

    def contains(self, p):
        """Check if this bounding box contains a given point."""
        point = Point(p)

        return (
			self.xmin <= point.x <= self.xmax 
			and self.ymin <= point.y <= self.ymax
		)

    def to_list(self):
        """Convert the bounding box to a 2d list format."""
        return [[self.xmin, self.xmax], [self.ymin, self.ymax]]


class Point():
    """Describes a point in R^3."""
    def __init__(self,coords):
        self.x = coords[0]
        self.y = coords[1]
        self.z = coords[2]
