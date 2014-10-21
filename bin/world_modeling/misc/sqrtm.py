from scipy import linalg
from scipy import diag
from scipy import sqrt
from scipy import ones
from numpy import dot


def sqrtm(M):
    """
    calculate the square root of a matrix.

    NOTE: maybe it is not feasible for all kind of matrices.
          maybe it only works for positiv semidefinit matrices.
    http://mail.scipy.org/pipermail/numpy-discussion/2003-September/002256.html
    """
    U, S, VT = linalg.svd(M)
    D = diag(sqrt(S))
    return dot(dot(U,D),VT)


def _assert_sqrt(original, sqrt_original):
    """
    make sure the sqrtm algo works
    """
    assert original.shape == sqrt_original.shape
    result = dot(sqrt_original, sqrt_original)
    cols, rows = original.shape
    for i in range(cols):
        for j in range(rows):
            print 'orig     ', original[i, j]
            print 'sqrt*sqrt', result[i, j]
            print 'absdiff', abs(original[i, j] - result[i, j])
            assert abs(original[i, j] - result[i, j]) < 0.01


def _test():
    m = ones((3, 3)) * 4
    print "m"
    print m

    res = sqrtm(m)
    print "sqrtm(m)"
    print res

    print "dot(res, res)"
    res = dot(res, res)
    print res

    for row in res:
        for e in row:
            assert(abs(e - 4.) < 0.001)


if __name__ == '__main__':
    _test()
