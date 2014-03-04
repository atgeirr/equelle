
#include <iostream>

#include <vector>

#include <opm/core/utility/parameters/ParameterGroup.hpp>
#include <opm/core/grid.h>
#include <opm/core/grid/GridManager.hpp>
#include <opm/core/utility/ErrorMacros.hpp>

#include "EquelleRuntimeCUDA.hpp"
#include "DeviceGrid.hpp"
#include "CollOfIndices.hpp"
#include "CollOfScalar.hpp"


using namespace equelleCUDA;



int compare(CollOfScalar scal, double sol[], 
	    int sol_size,
	    std::string test);
int collOfScalarTest(EquelleRuntimeCUDA* er);

int inputDomainTest(EquelleRuntimeCUDA* er);
int inputVectorComp(std::vector<int> host, std::string test);

int scalar_test(EquelleRuntimeCUDA* er);


//test_suite* init_unit_test_suite( int argc, char* argv[] )
//{
int main( int argc, char** argv) {
    
    Opm::parameter::ParameterGroup param( argc, argv, false);
    EquelleRuntimeCUDA er(param);
    
    DeviceGrid dg(er.getGrid());
    
    //if ( inputDomainTest(&er) ) {
    //    return 1;
    //}

    if ( collOfScalarTest(&er) ) {
	return 1;
    }
    
    if ( scalar_test(&er) ) {
     	return 1;
    }

    return 0;
}



int collOfScalarTest(EquelleRuntimeCUDA* er) {
    
    CollOfScalar a = er->inputCollectionOfScalar("a", er->allCells());
    double a_full_sol[] = {0,10,20,30,40,50,60,70,80,90,100,110};
    int a_full_size = 12;
    if ( compare(a, a_full_sol, a_full_size, "inputCollectionOfScalar(a)") ) {
	return 1;
    }

    CollOfScalar b = er->inputCollectionOfScalar("b", er->allCells());
    double b_full_sol[] = {124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124, 124.124};
    int b_full_size = 12;
    if ( compare(b, b_full_sol, b_full_size, "inputCollectionOfScalar(b)") ) {
	return 1;
    }

    CollOfScalar faces = er->inputCollectionOfScalar("faces", er->allFaces());
    double faces_sol[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    for (int i = 0; i < 31; i++) {
	faces_sol[i] *= i*10;
    }
    int faces_size = 31;
    if ( compare(faces, faces_sol, faces_size, "inputaCollectionOfScalar(faces)") ) {
	return 1;
    }
    
    return 0;
}



int scalar_test(EquelleRuntimeCUDA* er) {
    double scal_1 = er->inputScalarWithDefault("scal1", 3.14);
    if ( scal_1 != 2.7182 ) {
	std::cout << "Error in valsOnGrid.cpp - testing inputScalarWithDefault\n";
	std::cout << "\tShould find value from file.\n";
	std::cout << "\t scal_1 is " << scal_1 << " but should be 2.7182\n";
	return 1;
    }
    double scal_2 = er->inputScalarWithDefault("scal2", 164.93032);
    if ( scal_2 != 164.93032) {
	std::cout << "Error in valsOnGrid.cpp - testing inputScalarWithDefault\n";
	std::cout << "\tShould take default value 164.93032 but scal_2 is " << scal_2 << std::endl;
	return 1;
    }
    return 0;
}

int compare(CollOfScalar scal, double sol[], 
	    int sol_size,
	    std::string test) 
{ 
    // Test size:
    if ( scal.size() != sol_size ) {
	std::cout << "Error in valsOnGrid.cpp - testing " << test << "\n";
	std::cout << "\tThe collection is of wrong size!\n";
	std::cout << "\tSize is " << scal.size() << " but should be " << sol_size << "\n";
	return 1;
    }
    
    // Testing indices
    std::vector<double> host = scal.copyToHost();
    std::cout << "CollOfScalar " << test << " is the following:\n";
    bool correct = true;
    for (int i = 0; i < host.size(); ++i) {
	std::cout << host[i] << " ";
	if (i < sol_size) {
	    if (host[i] != sol[i]) {
		correct = false;
	    }
	}
    }
    if (correct) {
	std::cout << "\n\tThis is correct\n";
    } else {
	std::cout << "\n\tThis is wrong\n";
	std::cout << "Error in valsOnGrid.cpp - testing " << test << "\n";
	std::cout << "\tThe indices in the collection is wrong\n";
	return 1;
    }

    return 0;

}

/*
int inputDomainTest(EquelleRuntimeCUDA* er) {

    CollOfFace in_face = er->inputDomainSubsetOf("ind", er->allFaces());
    if ( inputVectorComp(in_face.stdToHost(), "allFaces()") ) {
	return 1;
    }
    CollOfCell in_cell = er->inputDomainSubsetOf("ind", er->allCells());
    if ( inputVectorComp(in_cell.stdToHost(), "allCells()") ) {
	return 1;
    }


    return 0;
}
*/


int inputVectorComp(std::vector<int> host, std::string test) {
    if ( host.size() != 3) {
	std::cout << "Error in valsOnGrid.cpp - testing inputDomainSubsetOf(" << test << ")\n";
	std::cout << "\tThe collection is of wrong size!\n";
	std::cout << "\tSize is " << host.size() << " should be 3\n";
	return 1;
    }

    bool correct = true;
    std::cout << "Input indices:\n";
    for( int i = 0; i < host.size(); i++) {
	std::cout << host[i] << " ";
	if ( host[i] != i) {
	    correct = false;
	}
    }
    std::cout << "\n";
    if ( !correct ) {
	std::cout << "This is wrong\n";
	std::cout << "Error in valsOnGrid.cpp - testing " << test << "\n";
	std::cout << "\tThe indices in the collection is wrong\n";
	std:: cout << "\tShould be 0 1 2\n";
	return 1;
    }

    return 0;
}
