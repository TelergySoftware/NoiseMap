#include <iostream>
#include <fstream>
#include "FastNoise.h"
#include <string>
#include <time.h>
#include <math.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

using namespace std;
using namespace cv;
using namespace boost::python;
namespace np = boost::python::numpy;

class NoiseMap {
    public:

        NoiseMap(int width, int height) { // Constructor needs the initial width and height of the map

            if(width <= 0) {this->width = 1;} // It can't be lesser than 1
            else {this->width = width;}

            if(height <= 0) {this->height = 1;} // It can't be lesser than 1
            else {this->height = height;}

            this->shape = boost::python::make_tuple(width, height); // Set initial shape
            this->objMatrix = np::zeros(this->shape, this->dtype); // Create a matrix with the tuple shape
            this->gen();
        }

        void setSeed(int seed) { // Set the seed to a given int
            this->seed = seed;
            this->gen();
        }
        void setSeed() { // Set a random seed
            this->seed = (int)(rand() * time(NULL));
            this->gen();
        }

        void setDeepness(float deepness) { // Set the map deepnes
            this->deepness = deepness;
            this->gen();
        }

        void setPower(float power) { // Set the power to a given float number
            if(power <= 0) {this->power = 0.0001;} // if power is less or equal to 0, power receives 0.0001
            else {
                this->power = power;
                this->gen();
            }
        }

        void setDetails(float details) { // Set the details to a given float number
            if(details <= 0) {this->details = 0.0001;} // if details is less or equal to 0, details receives 0.0001
            else {
                this->details = details;
                this->gen();
            }
        }

        void setShape(int width, int height) { // Set shape to a tuple of the form (width, height)
            if(width <= 0) {this->width = 1;} // width has to be greater than 0
            else {this->width = width;}

            if(height <= 0) {this->height = 1;} // height has to be greater than 0
            else {this->height = height;}

            this->shape = boost::python::make_tuple(width, height); // shape = (width, height)
            this->objMatrix = np::zeros(this->shape, this->dtype); // create a new map with the new shape
            this->gen(); // generate the new map with the same seed
        }

        int getSeed() {return this->seed;} // return seed value
        int getDeepness() {return this->deepness;} // return map deepness value
        float getPower() {return this->power;} // return power value
        float getDetails() {return this->details;} // return details value

        boost::python::tuple getShape() {return this->shape;} // return shape tuple
        np::ndarray getNumpyMap() {return this->objMatrix;} // return numpy ndarray

        void toOBJ(string name) {
            cout << "Converting to .OBJ" << endl;
            ofstream myOBJ;
            myOBJ.open(name + ".obj");
            myOBJ << "# This is an OBJ file created with MTH editor.\n\n";
            myOBJ << "o" << " " << name << "\n\n";
            myOBJ << "# Vertices: \n\n";
            for(int y = 0; y < this->height; y++) {
                for(int x = 0; x < this->width; x++) {
                    myOBJ << "v " << x << " " << boost::python::extract<char const*>(str(this->objMatrix[x][y])) << " " << y << "\n";
                }
            }
            myOBJ << "\n" << "# Faces:\n\n";
            for (int i=1; i <= pow(this->width, 2); i++) {
                if (i+this->width < pow(this->width, 2)) {
                    if (i % this->width == 0) {
                        continue;
                    }
                    else {
                        myOBJ << "f " << i << " " << i+1 << " " << i+this->width+1 << " " << i+this->width << "\n";
                    }
                }
                else {
                    break;
                }
            }
        }

        void gen() {
            this->myNoise.SetSeed(this->seed);
            for(int y = 0; y < this->height; y++) {
                for(int x = 0; x < this->width; x++) {
                    float height = pow((this->myNoise.GetNoise(x / this->details, y / this->details) + 1) / 2, this->power) * this->deepness;
                    this->objMatrix[x][y] = height;
                }
            }
        }

    private:
        FastNoise myNoise; // noise object, used to get the simplex noise in order to generate the height map
        int width, height; // height map width and height
        float deepness = 100; // height map max height
        int seed = 0; // seed used to randomize the map
        float power = 1, details = 1; // change map's chacracteristics
        boost::python::tuple shape = boost::python::make_tuple(1, 1); // initialize tuple as (1,1)
        np::dtype dtype = np::dtype::get_builtin<float>(); // type used in the numpy ndarray, float
        np::ndarray objMatrix = np::zeros(shape, dtype); // matrix containing the vertex data
};

BOOST_PYTHON_MODULE(NoiseModule) {  // python wrapper
    Py_Initialize();
    boost::python::numpy::initialize();

    class_<NoiseMap>("NoiseMap", init<int, int>())
        .def("setShape", &NoiseMap::setShape, boost::python::args("width", "height"))
        .def("setDeepness", &NoiseMap::setDeepness, boost::python::arg("deepness"))
        .def("setPower", &NoiseMap::setPower, boost::python::arg("power"))
        .def("setDetails", &NoiseMap::setDetails, boost::python::arg("details"))
        .def("getSeed", &NoiseMap::getSeed)
        .def("getDeepness", &NoiseMap::getDeepness)
        .def("getPower", &NoiseMap::getPower)
        .def("getDetails", &NoiseMap::getDetails)
        .def("getShape", &NoiseMap::getShape)
        .def("getNumpyMap", &NoiseMap::getNumpyMap)
        .def("toOBJ", &NoiseMap::toOBJ, boost::python::arg("obj_name"));
}
