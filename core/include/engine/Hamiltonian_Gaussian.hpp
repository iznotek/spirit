#pragma once
#ifndef HAMILTONIAN_GAUSSIAN_H
#define HAMILTONIAN_GAUSSIAN_H

#include <vector>

#include "Core_Defines.h"
#include "Vectormath_Defines.hpp"
#include "Hamiltonian.hpp"
#include "Geometry.hpp"

namespace Engine
{
	/*
	The Gaussian Hamiltonian is meant for testing purposes and demonstrations. Spins do not interact.
	A set of gaussians is summed with weight-factors so as to create an arbitrary energy landscape.
	E = sum_i^N a_i exp( -l_i^2(m)/(2sigma_i^2) ) where l_i(m) is the distance of m to the gaussian i,
		a_i is the gaussian amplitude and sigma_i the width
	*/
	class Hamiltonian_Gaussian : public Hamiltonian
	{
	public:
		// Constructor
		Hamiltonian_Gaussian(
			std::vector<scalar> amplitude, std::vector<scalar> width, std::vector<Vector3> center
		);

		// General Hamiltonian functions
		void Hessian(const std::vector<Vector3> & spins, MatrixX & hessian) override;
		void Effective_Field(const std::vector<Vector3> & spins, std::vector<Vector3> & field) override;
		scalar Energy(const std::vector<Vector3> & spins) override;
		std::vector<scalar> Energy_Array(const std::vector<Vector3> & spins) override;
		//std::vector<std::vector<scalar>> Energy_Array_per_Spin(std::vector<scalar> & spins) override;

		// Hamiltonian name as string
		const std::string& Name() override;

		// Parameters of the energy landscape
		int n_gaussians;
		std::vector<scalar> amplitude;
		std::vector<scalar> width;
		std::vector<Vector3> center;
	};
}
#endif