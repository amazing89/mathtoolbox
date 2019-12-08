#include <Eigen/LU>
#include <cmath>
#include <mathtoolbox/rbf-interpolation.hpp>

using Eigen::FullPivLU;
using Eigen::Map;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

namespace
{
    inline VectorXd SolveLinearSystem(const MatrixXd& A, const VectorXd& y)
    {
        FullPivLU<MatrixXd> lu(A);
        return lu.solve(y);
    }
} // namespace

mathtoolbox::RbfInterpolation::RbfInterpolation(const std::shared_ptr<AbstractRbfKernel> rbf_kernel)
    : m_rbf_kernel(rbf_kernel)
{
}

void mathtoolbox::RbfInterpolation::SetData(const Eigen::MatrixXd& X, const Eigen::VectorXd& y)
{
    assert(y.rows() == X.cols());
    this->X = X;
    this->y = y;
}

void mathtoolbox::RbfInterpolation::ComputeWeights(bool use_regularization, double lambda)
{
    const int dim = y.rows();

    MatrixXd Phi = MatrixXd::Zero(dim, dim);
    for (int i = 0; i < dim; ++i)
    {
        for (int j = i; j < dim; ++j)
        {
            Phi(i, j) = Phi(j, i) = GetRbfValue(X.col(i), X.col(j));
        }
    }

    const MatrixXd A = use_regularization ? Phi.transpose() * Phi + lambda * MatrixXd::Identity(dim, dim) : Phi;
    const VectorXd b = use_regularization ? Phi.transpose() * y : y;

    w = SolveLinearSystem(A, b);
}

double mathtoolbox::RbfInterpolation::GetValue(const VectorXd& x) const
{
    const int dim = w.rows();

    double result = 0.0;
    for (int i = 0; i < dim; ++i)
    {
        result += w(i) * GetRbfValue(x, X.col(i));
    }

    return result;
}

double mathtoolbox::RbfInterpolation::GetRbfValue(double r) const
{
    return m_rbf_kernel->EvaluateValue(r);
}

double mathtoolbox::RbfInterpolation::GetRbfValue(const VectorXd& xi, const VectorXd& xj) const
{
    assert(xi.rows() == xj.rows());
    return GetRbfValue((xj - xi).norm());
}
