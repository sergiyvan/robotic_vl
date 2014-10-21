#ifndef SPEEDTESTARMADILLO_H
#define SPEEDTESTARMADILLO_H


#include <iostream>

#include <inttypes.h>
#include <armadillo>

void speedtest(int N, int size) {

	using namespace std;
	using namespace arma;

    cout << "========================================" << endl;
	cout << "runs:" << N << "size:" << size << endl;
    cout << "========================================" << endl;

	wall_clock timer;

	mat A = randu(size, size);
	mat B = randu(size, size);
	mat C = randu(size, size);
	mat D = randu(size, size);

	mat Z = zeros(size, size);

	timer.tic();
	for (int i = 0; i < N; ++i) {
		Z = A + B; //  or Z = A+B+C ... etc
	}
	cout << "time taken " << timer.toc() / double(N);
	cout << " for addition" << endl;
    cout << "========================================" << endl;

	timer.tic();
	for (int i = 0; i < N; ++i) {
		Z = A + B + C + D;
	}
	cout << "time taken = " << timer.toc() / double(N);
	cout << " for multiple addition" << endl;
    cout << "========================================" << endl;

	timer.tic();
	for (int i = 0; i < N; ++i) {
		Z = A * B * C * D;
	}
	cout << "time taken = " << timer.toc() / double(N);
	cout << " for multiple multiplication" << endl;
    cout << "========================================" << endl;

	timer.tic();
	for (int i = 0; i < N; ++i) {
		B.row(size - 1) = A.row(0);
	}
	cout << "time taken = " << timer.toc() / double(N);
	cout << " for submatrix manipulation" << endl;
    cout << "========================================" << endl;

	colvec p = randu(size);
	colvec q = randu(size);
	colvec r = randu(size);
	double tmp;
	timer.tic();
	for (int i = 0; i < N; ++i) {
		tmp = as_scalar(trans(p) * inv(diagmat(q)) * r);
	}
	cout << "time taken = " << timer.toc() / double(N);
	cout << " for multi operation" << endl;
    cout << "========================================" << endl;

	//timer.tic();
	//Z = chol(A);
	//cout << "time taken = " << timer.toc() / double(N) << endl;
	//cout << "chol" << endl;

	timer.tic();
	Z = inv(A);
	cout << "time taken = " << timer.toc() / double(N);
	cout << " for matrix inv" << endl;
    cout << "========================================" << endl;

	// avoind -wunused warning
	tmp = tmp * tmp;
}



/**
 * @brief Armadillo example1.
 */
void example1() {
	using namespace std;
	using namespace arma;
	cout << "Armadillo version: " << arma_version::as_string() << endl;

	// directly specify the matrix size (elements are uninitialised)
	mat A(2, 3);

	// .n_rows = number of rows    (read only)
	// .n_cols = number of columns (read only)
	cout << "A.n_rows = " << A.n_rows << endl;
	cout << "A.n_cols = " << A.n_cols << endl;

	// directly access an element (indexing starts at 0)
	A(1, 2) = 456.0;

	A.print("A:");

	// scalars are treated as a 1x1 matrix,
	// hence the code below will set A to have a size of 1x1
	A = 5.0;
	A.print("A:");

	// if you want a matrix with all elements set to a particular value
	// the .fill() member function can be used
	A.set_size(3, 3);
	A.fill(5.0);
	A.print("A:");


	mat B;

	// endr indicates "end of row"
	B << 0.555950 << 0.274690 << 0.540605 << 0.798938 << endr
	  << 0.108929 << 0.830123 << 0.891726 << 0.895283 << endr
	  << 0.948014 << 0.973234 << 0.216504 << 0.883152 << endr
	  << 0.023787 << 0.675382 << 0.231751 << 0.450332 << endr;

	// print to the cout stream
	// with an optional string before the contents of the matrix
	B.print("B:");

	// the << operator can also be used to print the matrix
	// to an arbitrary stream (cout in this case)
	cout << "B:" << endl << B << endl;

	// save to disk
	B.save("B.txt", raw_ascii);

	// load from disk
	mat C;
	C.load("B.txt");

	C += 2.0 * B;
	C.print("C:");


	// submatrix types:
	//
	// .submat(first_row, first_column, last_row, last_column)
	// .row(row_number)
	// .col(column_number)
	// .cols(first_column, last_column)
	// .rows(first_row, last_row)

	cout << "C.submat(0,0,3,1) =" << endl;
	cout << C.submat(0, 0, 3, 1) << endl;

	// generate the identity matrix
	mat D = eye<mat>(4, 4);

	D.submat(0, 0, 3, 1) = C.cols(1, 2);
	D.print("D:");

	// transpose
	cout << "trans(B) =" << endl;
	cout << trans(B) << endl;

	// maximum from each column (traverse along rows)
	cout << "max(B) =" << endl;
	cout << max(B) << endl;

	// maximum from each row (traverse along columns)
	cout << "max(B,1) =" << endl;
	cout << max(B, 1) << endl;

	// maximum value in B
	cout << "max(max(B)) = " << max(max(B)) << endl;

	// sum of each column (traverse along rows)
	cout << "sum(B) =" << endl;
	cout << sum(B) << endl;

	// sum of each row (traverse along columns)
	cout << "sum(B,1) =" << endl;
	cout << sum(B, 1) << endl;

	// sum of all elements
	cout << "sum(sum(B)) = " << sum(sum(B)) << endl;
	cout << "accu(B)     = " << accu(B) << endl;

	// trace = sum along diagonal
	cout << "trace(B)    = " << trace(B) << endl;

	// random matrix -- values are uniformly distributed in the [0,1] interval
	mat E = randu<mat>(4, 4);
	E.print("E:");

	cout << endl;

	// row vectors are treated like a matrix with one row
	rowvec r;
	r << 0.59499 << 0.88807 << 0.88532 << 0.19968;
	r.print("r:");

	// column vectors are treated like a matrix with one column
	colvec q;
	q << 0.81114 << 0.06256 << 0.95989 << 0.73628;
	q.print("q:");

	// dot or inner product
	cout << "as_scalar(r*q) = " << as_scalar(r * q) << endl;


	// outer product
	cout << "q*r =" << endl;
	cout << q* r << endl;

	// multiply-and-accumulate operation
	// (no temporary matrices are created)
	cout << "accu(B % C) = " << accu(B % C) << endl;

	// sum of three matrices (no temporary matrices are created)
	mat F = B + C + D;
	F.print("F:");

	// imat specifies an integer matrix
	imat AA;
	imat BB;

	AA << 1 << 2 << 3 << endr << 4 << 5 << 6 << endr << 7 << 8 << 9;
	BB << 3 << 2 << 1 << endr << 6 << 5 << 4 << endr << 9 << 8 << 7;

	// comparison of matrices (element-wise)
	// output of a relational operator is a umat
	umat ZZ = (AA >= BB);
	ZZ.print("ZZ =");


	// 2D field of arbitrary length row vectors
	// (fields can also store abitrary objects, e.g. instances of std::string)
	field<rowvec> xyz(3, 2);

	xyz(0, 0) = randu(1, 2);
	xyz(1, 0) = randu(1, 3);
	xyz(2, 0) = randu(1, 4);
	xyz(0, 1) = randu(1, 5);
	xyz(1, 1) = randu(1, 6);
	xyz(2, 1) = randu(1, 7);

	cout << "xyz:" << endl;
	cout << xyz << endl;


	// cubes ("3D matrices")
	cube Q( B.n_rows, B.n_cols, 2 );

	Q.slice(0) = B;
	Q.slice(1) = 2.0 * B;

	Q.print("Q:");
}

void example2() {
	using namespace std;
	using namespace arma;
	cout << "Armadillo version: " << arma_version::as_string() << endl;

	mat A;

	A << 0.165300 << 0.454037 << 0.995795 << 0.124098 << 0.047084 << endr
	  << 0.688782 << 0.036549 << 0.552848 << 0.937664 << 0.866401 << endr
	  << 0.348740 << 0.479388 << 0.506228 << 0.145673 << 0.491547 << endr
	  << 0.148678 << 0.682258 << 0.571154 << 0.874724 << 0.444632 << endr
	  << 0.245726 << 0.595218 << 0.409327 << 0.367827 << 0.385736 << endr;

	A.print("A =");

	// determinant
	cout << "det(A) = " << det(A) << endl;

	// inverse
	cout << "inv(A) = " << endl << inv(A) << endl;


	//

	double k = 1.23;

	mat    B = randu<mat>(5, 5);
	mat    C = randu<mat>(5, 5);

	rowvec r = randu<rowvec>(5);
	colvec q = randu<colvec>(5);


	// examples of some expressions
	// for which optimised implementations exist

	// optimised implementation of a trinary expression
	// that results in a scalar
	cout << "as_scalar( r*inv(diagmat(B))*q ) = ";
	cout << as_scalar( r * inv(diagmat(B))*q ) << endl;

	// example of an expression which is optimised
	// as a call to the dgemm() function in BLAS:
	cout << "k*trans(B)*C = " << endl << k* trans(B)*C;


	// If you want to see a trace of how Armadillo
	// evaluates expressions, compile with the
	// ARMA_EXTRA_DEBUG macro defined.
	// This was designed to work with the GCC compiler,
	// but it may also work with other compilers
	// if you have the Boost libraries installed
	// and told Armadillo to use them.
	//
	// Example for GCC:
	// g++ example2.cpp -o example2 -larmadillo -DARMA_EXTRA_DEBUG
	//
	// Running example2 will now produce a truckload of messages,
	// so you may want to redirect the output to a log file.

}

void run() {

    int runs = 10000;
    int size;


    size = 4;
	speedtest(runs, size);

    size = 16;
	speedtest(runs, size);

    size = 50;
	speedtest(runs, size);


    size = 100;
	speedtest(runs, size);



	example1();
	example2();
}

#endif /* SPEEDTESTARMADILLO_H */
