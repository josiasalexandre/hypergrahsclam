#ifndef HYPERGRAPHSLAM_EDGE_VELODYNE_6D_CALIBRATION_HPP
#define HYPERGRAPHSLAM_EDGE_VELODYNE_6D_CALIBRATION_HPP

#include <g2o/core/base_multi_edge.h>
#include <g2o/types/slam2d/vertex_se3.h>

namespace g2o {

  /**
   * \brief scanmatch measurement that also calibrates an offset for the laser
   */
class EdgeVelodyne3DCalibration : public BaseMultiEdge<3, Eigen::Isometry3d> {

    protected:

        // precomputed inverse measurement
        Eigen::Isometry3d _inverseMeasurement;

        // the displacement
        static Eigen::Isometry3d _displacement;

    public:

        // eigen new operator
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        // the base constructor
        EdgeVelodyneCalibration() {

            // resize the base edge
            resize(3);

        }

        // the main error function
        void computeError() {

            // the pose estimates
            const VertexSE2* v1 = static_cast<const VertexSE2*>(_vertices[0]);
            const VertexSE2* v2 = static_cast<const VertexSE2*>(_vertices[1]);

            // the velodyne sensor offset
            const VertexSE2* sensor_offset = static_cast<const VertexSE2*>(_vertices[2]);

            // direct access
            const SE2& x1 = v1->estimate();
            const SE2& x2 = v2->estimate();
            const SE2& velodyne = sensor_offset->estimate();

            // compute the error
            SE2 delta = _inverseMeasurement * (((x1 * velodyne) * _displacement).inverse() * ((x2 * velodyne) * _displacement));
            // SE2 delta = _inverseMeasurement * (((x1 * velodyne).inverse()) * (x2 * velodyne));

            // save the error in an Eigen Vector
            _error = delta.toVector();

        }

        void setMeasurement(const SE2& m){

            // set the measurement value
            _measurement = m;

            // set the measurement inverse inverse
            _inverseMeasurement = m.inverse();

        }

        virtual double initialEstimatePossible(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* to) {

            // try to find the offset vertex in the std set<Vertex*> "from"
            bool need_the_laser_offset = 1 == from.count(_vertices[2]);

            if (need_the_laser_offset && ((from.count(_vertices[0]) == 1 && to == _vertices[1]) || ((from.count(_vertices[1]) == 1 && to == _vertices[0])))) {

                return 1.0;

            }

            return -1.0;

        }

        virtual void initialEstimate(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* to) {

            (void) to;

            // the start pose
            VertexSE2* vi = static_cast<VertexSE2*>(_vertices[0]);

            // the target pose
            VertexSE2* vj = static_cast<VertexSE2*>(_vertices[1]);

            // the sensor offset
            VertexSE2* l  = static_cast<VertexSE2*>(_vertices[2]);

            if (from.count(l) == 0) {

                return;

            }

            if (from.count(vi) == 1) {

                vj->setEstimate(vi->estimate() * l->estimate() * measurement() * l->estimate().inverse());

            } else {

                vi->setEstimate(vj->estimate() * l->estimate() * _inverseMeasurement * l->estimate().inverse());

            }

        }

        //
        virtual bool read(std::istream& is) {

            // helper
            Eigen::Vector3d p;

            // parse the double values, considering an SE2
            is >> p(0) >> p(1) >> p(2);

            // update the measurement
            _measurement.fromVector(p);

            // precompute the inverse measurement
            _inverseMeasurement = measurement().inverse();

            for (int i = 0; i < information().rows(); ++i) {

                for (int j = i; j < information().cols(); ++j) {

                    // get the information values
                    is >> information()(i, j);

                    if (i != j) {

                        // considering the undirected graph it results in a symmetric matrix
                        information()(j, i) = information()(i, j);

                    }
                }

            }

            return true;

        }

        virtual bool write(std::ostream& os) const {

            // get the measurement in a Eigen vector
            Eigen::Vector3d p = measurement().toVector();

            // save it to the output stream
            os << p(0) << " " << p(1) << " " << p(2);

            for (int i = 0; i < information().rows(); ++i) {

                for (int j = i; j < information().cols(); ++j) {

                    // append the information matrix values
                    os << " " << information()(i, j);

                }

            }

            return os.good();

        }


};

// velodyne_x  0.145
// velodyne_y  0.0
// velodyne_z  0.48
// velodyne_roll   0.0
// velodyne_pitch  -0.0227
// velodyne_yaw    -0.01 # 0.09

// set the default displacement
g2o::SE2 EdgeVelodyneCalibration::_displacement = g2o::SE2(0.145, 0.0, -0.01);

} // end namespace

#endif
