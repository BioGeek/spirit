#pragma once
#ifndef UTILITY_CONFIGURATION_CHAIN_H
#define UTILITY_CONFIGURATION_CHAIN_H

#include "Spin_System_Chain.h"
#include <vector>

namespace Utility
{
	namespace Configuration_Chain
	{
		// Homogeneous rotation of all spins from configuration A to B for all images in a chain
		void Homogeneous_Rotation(std::shared_ptr<Data::Spin_System_Chain> c, std::vector<double> A, std::vector<double> B);

		// Homogeneous rotation of all spins from first to last configuration of the given configurations
		void Homogeneous_Rotation(std::shared_ptr<Data::Spin_System_Chain> c, int idx_1, int idx_2);

	};//end namespace Configurations
}//end namespace Utility

#endif
