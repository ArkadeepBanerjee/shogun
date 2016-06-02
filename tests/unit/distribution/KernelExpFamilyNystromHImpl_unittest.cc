/*
 * Copyright (c) The Shogun Machine Learning Toolbox
 * Written (w) 2016 Heiko Strathmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Shogun Development Team.
 */

#include <shogun/features/DenseFeatures.h>
#include <shogun/distributions/kernel_exp_family/impl/KernelExpFamilyImpl.h>
#include <shogun/distributions/kernel_exp_family/impl/KernelExpFamilyNystromHImpl.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/mathematics/Math.h>
#include <shogun/mathematics/eigen3.h>
#include <shogun/base/some.h>
#include <gtest/gtest.h>

using namespace shogun;
using namespace Eigen;

TEST(KernelExpFamilyNystromHImpl, fit_all_inds_equals_exact)
{
	index_t N=5;
	index_t D=3;
	auto ND=N*D;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	KernelExpFamilyNystromHImpl est_nystrom(X, sigma, lambda, ND);
	
	est.fit();
	est_nystrom.fit();
	
	// compare against full version
	auto result_nystrom=est_nystrom.get_alpha_beta();
	auto result=est.get_alpha_beta();
	
	ASSERT_EQ(result.vlen, ND+1);
	ASSERT_EQ(result_nystrom.vlen, ND+1);
	ASSERT(result.vector);
	ASSERT(result_nystrom.vector);
	
	
	for (auto i=0; i<ND+1; i++)
		EXPECT_NEAR(result[i], result_nystrom[i], 1e-10);
}

TEST(KernelExpFamilyNystromHImpl, fit_half_inds_shape)
{
	index_t N=5;
	index_t D=3;
	auto ND=N*D;
	index_t m=ND/2;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyNystromHImpl est(X, sigma, lambda, m);
	est.fit();
	
	auto alpha_beta=est.get_alpha_beta();
	ASSERT_EQ(alpha_beta.vlen, m+1);
	ASSERT(alpha_beta.vector);
}

TEST(KernelExpFamilyNystromHImpl, log_pdf_almost_all_inds_close_exact)
{
	index_t N=5;
	index_t D=3;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	auto m=N*D-1;
	KernelExpFamilyNystromHImpl est_nystrom(X, sigma, lambda, m);
	KernelExpFamilyImpl est(X, sigma, lambda);
	est_nystrom.fit();
	est.fit();
	
	SGVector<float64_t> x(D);
	for (auto i=0; i<D; i++)
		x[i]=CMath::randn_float();

	auto log_pdf = est.log_pdf(x);
	auto log_pdf_nystrom = est_nystrom.log_pdf(x);
	
	EXPECT_NEAR(log_pdf, log_pdf_nystrom, 0.3);
}

TEST(KernelExpFamilyNystromHImpl, grad_all_inds_equals_exact)
{
	index_t N=5;
	index_t D=3;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	auto m=N*D;
	KernelExpFamilyNystromHImpl est_nystrom(X, sigma, lambda, m);
	KernelExpFamilyImpl est(X, sigma, lambda);
	est_nystrom.fit();
	est.fit();
	
	SGVector<float64_t> x(D);
	for (auto i=0; i<D; i++)
		x[i]=CMath::randn_float();

	auto grad = est.grad(x);
	auto grad_nystrom = est_nystrom.grad(x);
	
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(grad[i], grad_nystrom[i], 1e-8);
}

TEST(KernelExpFamilyNystromHImpl, grad_almost_all_inds_close_exact)
{
	index_t N=5;
	index_t D=3;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	auto m=N*D-1;
	KernelExpFamilyNystromHImpl est_nystrom(X, sigma, lambda, m);
	KernelExpFamilyImpl est(X, sigma, lambda);
	est_nystrom.fit();
	est.fit();
	
	SGVector<float64_t> x(D);
	for (auto i=0; i<D; i++)
		x[i]=CMath::randn_float();

	auto grad = est.grad(x);
	auto grad_nystrom = est_nystrom.grad(x);
	
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(grad[i], grad_nystrom[i], 0.3);
}
