#pragma once
#ifndef UTILITY_IO_H
#define UTILITY_IO_H

#include <string>
#include <memory>
#include <istream>
#include <fstream>
#include <sstream>
#include <type_traits>

#include "Spin_System.h"
#include "Spin_System_Chain.h"
#include "Geometry.h"
#include "Hamiltonian_Isotropic.h"
#include "Hamiltonian_Anisotropic.h"
#include "Parameters_LLG.h"
#include "Parameters_GNEB.h"


namespace Utility
{
	namespace IO
	{
		// ======================== Configparser ========================
		// Note that due to the modular structure of the input parsers, input may be given in one or in separate files.
		// Input may be given incomplete. In this case a log entry is created and default values are used.
		void Log_Levels_from_Config(const std::string configFile);
		std::unique_ptr<Data::Spin_System> Spin_System_from_Config(const std::string configFile);
		std::unique_ptr<Data::Geometry> Geometry_from_Config(const std::string configFile);
		std::unique_ptr<Data::Parameters_LLG> LLG_Parameters_from_Config(const std::string configFile);
		std::unique_ptr<Data::Parameters_GNEB> GNEB_Parameters_from_Config(const std::string configFile);
		std::unique_ptr<Engine::Hamiltonian_Isotropic> Hamiltonian_Isotropic_from_Config(const std::string configFile, Data::Geometry geometry);
		std::unique_ptr<Engine::Hamiltonian_Anisotropic> Hamiltonian_Anisotropic_from_Config(const std::string configFile, Data::Geometry geometry);

		// ========================= Fileparser =========================
		void Read_Spin_Configuration(std::shared_ptr<Data::Spin_System> s, const std::string file);
		void Read_SpinChain_Configuration(std::shared_ptr<Data::Spin_System_Chain> c, const std::string file);
		//External_Field_from_File ....
		void Pairs_from_File(const std::string pairsFile, Data::Geometry geometry, int & nop,
			std::vector<std::vector<std::vector<int>>> & Exchange_indices, std::vector<std::vector<double>> & Exchange_magnitude,
			std::vector<std::vector<std::vector<int>>> & DMI_indices, std::vector<std::vector<double>> & DMI_magnitude, std::vector<std::vector<std::vector<double>>> & DMI_normal,
			std::vector<std::vector<std::vector<int>>> & BQC_indices, std::vector<std::vector<double>> & BQC_magnitude);

		// =========================== Saving Configurations ===========================
		// Append Spin_Configuration to file
		void Append_Spin_Configuration(std::shared_ptr<Data::Spin_System> & s, const int iteration, const std::string fileName);
		// Saves Spin_Chain_Configuration to file
		void Save_SpinChain_Configuration(std::shared_ptr<Data::Spin_System_Chain> & c, const std::string fileName);

		// =========================== Saving Energies ===========================
		void Write_Energy_Header(const std::string fileName);
		// Appends the current Energy of the current image with energy contributions, without header
		void Append_Energy(Data::Spin_System &s, const int iteration, const std::string fileName);
		// Saves Energies of all images with header and contributions
		void Save_Energies(Data::Spin_System_Chain & c, const int iteration, const std::string fileName);
		// Saves the Energies interpolated by the GNEB method
		void Save_Energies_Interpolated(Data::Spin_System_Chain & c, const std::string fileName);
		// Saves the energy contributions of every spin of an image
		void Save_Energies_Spins(Data::Spin_System_Chain & c, const std::string fileName);
		void Save_Forces(Data::Spin_System_Chain & c, const std::string fileName);


		// ========================= Saving Helpers =========================
		// Creates a new thread with String_to_File, which is immediately detached
		void Dump_to_File(const std::string text, const std::string name);
		// Takes a vector of strings of size "no" and dumps those into a file asynchronously
		void Dump_to_File(const std::vector<std::string> text, const std::string name, const int no);

		// Dumps the contents of the strings in text vector into file "name"
		void Strings_to_File(const std::vector<std::string> text, const std::string name, const int no);
		// Dumps the contents of the string 'text' into a file
		void String_to_File(const std::string text, const std::string name);
		// Appends the contents of the string 'text' onto a file
		void Append_String_to_File(const std::string text, const std::string name);

		// ========================= Other Helpers =========================
		// Convert an int to a formatted string
		std::string int_to_formatted_string(int in, int n = 6);

	};// end namespace IO
}// end namespace utility
#endif