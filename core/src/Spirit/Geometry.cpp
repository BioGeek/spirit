#include <Spirit/Geometry.h>
#include <data/State.hpp>
#include <engine/Vectormath.hpp>
#include <utility/Logging.hpp>
#include <utility/Exception.hpp>


void Geometry_Set_N_Cells(State * state, int n_cells_i[3])
{
    // The new number of spins
    auto n_cells = intfield{n_cells_i[0], n_cells_i[1], n_cells_i[2]};
    int nos = n_cells[0]*n_cells[1]*n_cells[2]*Geometry_Get_N_Basis_Atoms(state);

    // Deal with all systems in all chains
    for (auto& chain : state->collection->chains)
    {
        for (auto& system : chain->images)
        {
            int nos_old = system->nos;
            system->nos = nos;

            // Geometry
            auto ge = system->geometry;
            // TODO: spin_pos and atom_types should be generated by Geometry (atom types needs some consideration...)
            auto spin_pos = vectorfield(nos);
            Engine::Vectormath::Build_Spins(spin_pos, ge->basis_atoms, ge->translation_vectors, n_cells);
            intfield atom_types(nos, 0);
            *system->geometry = Data::Geometry(ge->basis, ge->translation_vectors,
                n_cells, ge->basis_atoms, ge->lattice_constant,
                spin_pos, atom_types);
            
            // Spins
            // TODO: ordering of spins should be considered -> write a Configurations function for this
            system->spins->resize(nos);
            for (int i=nos_old; i<nos; ++i) (*system->spins)[i] = Vector3{0, 0, 1};

            // Parameters
            // TODO: properly re-generate pinning
            system->llg_parameters->pinning->mask_unpinned = intfield(nos, 1);
        }
    }

    // Update convenience integers across everywhere
    state->nos = nos;
    // Deal with clipboard image of State
    if (state->clipboard_image)
    {
        auto& system = state->clipboard_image;
        int nos_old = system->nos;
        system->nos = nos;
        
        // Geometry
        auto ge = system->geometry;
        // TODO: spin_pos and atom_types should be generated by Geometry (atom types needs some consideration...)
        auto spin_pos = vectorfield(nos);
        Engine::Vectormath::Build_Spins(spin_pos, ge->basis_atoms, ge->translation_vectors, n_cells);
        intfield atom_types(nos, 0);
        *system->geometry = Data::Geometry(ge->basis, ge->translation_vectors,
            n_cells, ge->basis_atoms, ge->lattice_constant,
            spin_pos, atom_types);
        
        // Spins
        // TODO: ordering of spins should be considered -> write a Configurations function for this
        system->spins->resize(nos);
        for (int i=nos_old; i<nos; ++i) (*system->spins)[i] = Vector3{0, 0, 1};
        
        // Parameters
        // TODO: properly re-generate pinning
        system->llg_parameters->pinning->mask_unpinned = intfield(nos, 1);
	}
    // Deal with clipboard configuration of State
	if (state->clipboard_spins)
	{
		// TODO: the previous configuration should be extended, not overwritten
		state->clipboard_spins = std::shared_ptr<vectorfield>(new vectorfield(nos, { 0, 0, 1 }));
	}

    // TODO: the Hamiltonians may contain arrays that depend on system size

	Log(Utility::Log_Level::Warning, Utility::Log_Sender::API, "Set number of cells for all Systems: (" + std::to_string(n_cells[0]) + ", " + std::to_string(n_cells[1]) + ", " + std::to_string(n_cells[2]) + ")", -1, -1);
}

void Geometry_Set_Basis_Atoms(State *state, int n_atoms, float ** atoms)
{
    Log(Utility::Log_Level::Warning, Utility::Log_Sender::API, "Geometry_Set_Basis_Atoms is not yet implemented", -1, -1);
}

void Geometry_Set_Translation_Vectors(State *state, float ta[3], float tb[3], float tc[3])
{
    Log(Utility::Log_Level::Warning, Utility::Log_Sender::API, "Geometry_Get_Translation_Vectors is not yet implemented", -1, -1);
}



int Geometry_Get_NOS(State * state)
{
    return state->nos;
}

scalar * Geometry_Get_Spin_Positions( State * state, int idx_image, int idx_chain )
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        return (scalar *)image->geometry->spin_pos[0].data();
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return nullptr;
    }
}

int * Geometry_Get_Atom_Types( State * state, int idx_image, int idx_chain )
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers 
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
            
        return (int *)image->geometry->atom_types.data();
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return nullptr;
    }
}

void Geometry_Get_Bounds( State *state, float min[3], float max[3], int idx_image, int idx_chain )
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
            
        auto g = image->geometry;
        for (int dim=0; dim<3; ++dim)
        {
            min[dim] = (float)g->bounds_min[dim];
            max[dim] = (float)g->bounds_max[dim];
        }   
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }
}

// Get Center as array (x,y,z)
void Geometry_Get_Center(State *state, float center[3], int idx_image, int idx_chain)
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
            
        auto g = image->geometry;
        for (int dim=0; dim<3; ++dim)
        {
            center[dim] = (float)g->center[dim];
        }
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }
}

void Geometry_Get_Cell_Bounds( State *state, float min[3], float max[3], int idx_image, int idx_chain )
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
            
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        auto g = image->geometry;
        for (int dim=0; dim<3; ++dim)
        {
            min[dim] = (float)g->cell_bounds_min[dim];
            max[dim] = (float)g->cell_bounds_max[dim];
        }
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }   
}

// Get basis vectors ta, tb, tc
void Geometry_Get_Basis_Vectors( State *state, float a[3], float b[3], float c[3], 
                                 int idx_image, int idx_chain )
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        auto g = image->geometry;
        for (int dim=0; dim<3; ++dim)
        {
            a[dim] = (float)g->basis[dim][0];
            b[dim] = (float)g->basis[dim][1];
            c[dim] = (float)g->basis[dim][2];
        }
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }
}

// TODO: Get basis atoms
// void Geometry_Get_Basis_Atoms(State *state, float * n_atoms, float ** atoms)
// {
//     auto g = state->active_image->geometry;
//     *n_atoms = g->n_spins_basic_domain;
// }

// Get number of atoms in a basis cell
int Geometry_Get_N_Basis_Atoms(State *state, int idx_image, int idx_chain)
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        return image->geometry->n_spins_basic_domain;
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return false;
    }
}

// Get number of basis cells in the three translation directions
void Geometry_Get_N_Cells(State *state, int n_cells[3], int idx_image, int idx_chain)
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        auto g = image->geometry;
        n_cells[0] = g->n_cells[0];
    	n_cells[1] = g->n_cells[1];
    	n_cells[2] = g->n_cells[2];
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }
}

// Get translation vectors ta, tb, tc
void Geometry_Get_Translation_Vectors( State *state, float ta[3], float tb[3], float tc[3], 
                                       int idx_image, int idx_chain )
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        auto g = image->geometry;
        for (int dim=0; dim<3; ++dim)
        {
            ta[dim] = (float)g->translation_vectors[0][dim];
            tb[dim] = (float)g->translation_vectors[1][dim];
            tc[dim] = (float)g->translation_vectors[2][dim];
        }
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
    }
}

int Geometry_Get_Dimensionality(State * state, int idx_image, int idx_chain)
{
    try
    {
    	std::shared_ptr<Data::Spin_System> image;
    	std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
    	auto g = image->geometry;
    	return g->dimensionality;
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return 0;
    }
}


int Geometry_Get_Triangulation( State * state, const int ** indices_ptr, int n_cell_step, 
                                int idx_image, int idx_chain )
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
      
        // TODO: we should also check if idx_image < 0 and log the promotion to idx_active_image
        
        auto g = image->geometry;
        auto& triangles = g->triangulation(n_cell_step);
        if (indices_ptr != nullptr) {
            *indices_ptr = reinterpret_cast<const int *>(triangles.data());
        }
        return triangles.size();
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return 0;
    }
}

int Geometry_Get_Tetrahedra( State * state, const int ** indices_ptr, int n_cell_step, 
                             int idx_image, int idx_chain )
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        from_indices(state, idx_image, idx_chain, image, chain);

        auto g = image->geometry;
        auto& tetrahedra = g->tetrahedra(n_cell_step);
        if (indices_ptr != nullptr) {
            *indices_ptr = reinterpret_cast<const int *>(tetrahedra.data());
        }
        return tetrahedra.size();
    }
    catch( ... )
    {
        Utility::Handle_Exception( idx_image, idx_chain );
        return 0;        
    }
}