#include <Spirit/Hamiltonian.h>

#include <data/State.hpp>
#include <data/Spin_System.hpp>
#include <data/Spin_System_Chain.hpp>
#include <engine/Vectormath.hpp>
#include <engine/Neighbours.hpp>
#include <engine/Hamiltonian_Heisenberg.hpp>
#include <engine/Hamiltonian_Gaussian.hpp>
#include <utility/Constants.hpp>
#include <utility/Logging.hpp>
#include <utility/Exception.hpp>

#include <fmt/format.h>

using namespace Utility;

/*------------------------------------------------------------------------------------------------------ */
/*---------------------------------- Set Parameters ---------------------------------------------------- */
/*------------------------------------------------------------------------------------------------------ */

void Hamiltonian_Set_Boundary_Conditions(State *state, const bool * periodical, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        image->Lock();
        try
        {
            image->hamiltonian->boundary_conditions[0] = periodical[0];
            image->hamiltonian->boundary_conditions[1] = periodical[1];
            image->hamiltonian->boundary_conditions[2] = periodical[2];
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }
        image->Unlock();

        Log( Utility::Log_Level::Info, Utility::Log_Sender::API,
            fmt::format("Set boundary conditions to {} {} {}", periodical[0], periodical[1], periodical[2]), 
            idx_image, idx_chain );
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_mu_s(State *state, float mu_s, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        image->Lock();
        
        try
        {
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
                for (auto& m : ham->mu_s) m = mu_s;
                Log(Utility::Log_Level::Info, Utility::Log_Sender::API,
                    fmt::format("Set mu_s to {}", mu_s), idx_image, idx_chain);
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
                    "mu_s cannot be set on " + image->hamiltonian->Name(), idx_image, idx_chain );
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }
        
        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_Field(State *state, float magnitude, const float * normal, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        // Lock mutex because simulations may be running
        image->Lock();
        
        try
        {
            // Set
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();

                // Normals
                Vector3 new_normal{normal[0], normal[1], normal[2]};
                new_normal.normalize();
                
                // Into the Hamiltonian
                ham->external_field_magnitude = magnitude * Constants::mu_B;
                ham->external_field_normal = new_normal;

                // Update Energies
                ham->Update_Energy_Contributions();

                Log( Utility::Log_Level::Info, Utility::Log_Sender::API,
                    fmt::format("Set external field to {}, direction ({}, {}, {})", magnitude, normal[0], normal[1], normal[2]),
                    idx_image, idx_chain );
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
                    "External field cannot be set on " + image->hamiltonian->Name(), idx_image, idx_chain );
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }
        
        // Unlock mutex
        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_Anisotropy( State *state, float magnitude, const float * normal, 
                                 int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );

        image->Lock();

        try
        {
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
                int nos = image->nos;
                int n_cell_atoms = image->geometry->n_cell_atoms;

                // Indices and Magnitudes
                intfield new_indices(n_cell_atoms);
                scalarfield new_magnitudes(n_cell_atoms);
                for (int i = 0; i<n_cell_atoms; ++i)
                {
                    new_indices[i] = i;
                    new_magnitudes[i] = magnitude;
                }
                // Normals
                Vector3 new_normal{ normal[0], normal[1], normal[2] };
                new_normal.normalize();
                vectorfield new_normals(nos, new_normal);

                // Into the Hamiltonian
                ham->anisotropy_indices = new_indices;
                ham->anisotropy_magnitudes = new_magnitudes;
                ham->anisotropy_normals = new_normals;

                // Update Energies
                ham->Update_Energy_Contributions();

                Log( Utility::Log_Level::Info, Utility::Log_Sender::API,
                    fmt::format("Set anisotropy to {}, direction ({}, {}, {})", magnitude, normal[0], normal[1], normal[2]),
                    idx_image, idx_chain );
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
                        "Anisotropy cannot be set on " + image->hamiltonian->Name(), idx_image, idx_chain);
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }

        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_Exchange(State *state, int n_shells, const float* jij, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        image->Lock();
        
        try
        {
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                // Update the Hamiltonian
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
                ham->exchange_shell_magnitudes = scalarfield(jij, jij + n_shells);
                ham->exchange_pairs_in         = pairfield(0);
                ham->exchange_magnitudes_in    = scalarfield(0);
                ham->Update_Interactions();

                std::string message = fmt::format("Set exchange to {} shells", n_shells);
                if (n_shells > 0) message += fmt::format(" Jij[0] = {}", jij[0]);
                Log(Utility::Log_Level::Info, Utility::Log_Sender::API, message, idx_image, idx_chain);
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
                        "Exchange cannot be set on " + image->hamiltonian->Name(), idx_image, idx_chain );
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }
                
        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_DMI(State *state, int n_shells, const float * dij, int chirality, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        image->Lock();

        try
        {
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                // Update the Hamiltonian
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
                ham->dmi_shell_magnitudes = scalarfield(dij, dij + n_shells);
                ham->dmi_shell_chirality  = chirality;
                ham->dmi_pairs_in         = pairfield(0);
                ham->dmi_magnitudes_in    = scalarfield(0);
                ham->dmi_normals_in       = vectorfield(0);
                ham->Update_Interactions();

                std::string message = fmt::format("Set dmi to {} shells", n_shells);
                if (n_shells > 0) message += fmt::format(" Dij[0] = {}", dij[0]);
                Log(Utility::Log_Level::Info, Utility::Log_Sender::API, message, idx_image, idx_chain);
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API, "DMI cannot be set on " + 
                        image->hamiltonian->Name(), idx_image, idx_chain );
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }

        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Set_DDI(State *state, float radius, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        image->Lock();

        try
        {
            if (image->hamiltonian->Name() == "Heisenberg")
            {
                auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();

                auto pairs = Engine::Neighbours::Get_Pairs_in_Radius(*image->geometry, radius);
                scalarfield magnitudes(0);
                vectorfield normals(0);
                scalar magnitude;
                Vector3 normal;
                for (auto& pair : pairs)
                {
                    Engine::Neighbours::DDI_from_Pair(*image->geometry, pair, magnitude, normal);
                    magnitudes.push_back(magnitude);
                    normals.push_back(normal);
                }
                ham->ddi_pairs = pairs;
                ham->ddi_magnitudes = magnitudes;
                ham->ddi_normals = normals;

                // Update the list of different contributions
                ham->Update_Energy_Contributions();

                Log( Utility::Log_Level::Info, Utility::Log_Sender::API, fmt::format("Set ddi radius to {}", radius), idx_image, idx_chain );
            }
            else
                Log( Utility::Log_Level::Warning, Utility::Log_Sender::API, "DDI cannot be set on " + 
                        image->hamiltonian->Name(), idx_image, idx_chain );
        }
        catch( ... )
        {
            spirit_handle_exception_api(idx_image, idx_chain);
        }

        image->Unlock();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

/*------------------------------------------------------------------------------------------------------ */
/*---------------------------------- Get Parameters ---------------------------------------------------- */
/*------------------------------------------------------------------------------------------------------ */

const char * Hamiltonian_Get_Name(State * state, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        return image->hamiltonian->Name().c_str();
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
        return nullptr;
    }
}

void Hamiltonian_Get_Boundary_Conditions(State *state, bool * periodical, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        periodical[0] = image->hamiltonian->boundary_conditions[0];
        periodical[1] = image->hamiltonian->boundary_conditions[1];
        periodical[2] = image->hamiltonian->boundary_conditions[2];
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Get_mu_s(State *state, float * mu_s, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
            for (int i=0; i<image->geometry->n_cell_atoms; ++i)
                mu_s[i] = (float)ham->mu_s[i];
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Get_Field(State *state, float * magnitude, float * normal, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();

            if (ham->external_field_magnitude > 0)
            {
                // Magnitude
                *magnitude = (float)(ham->external_field_magnitude / Constants::mu_B);

                // Normal
                normal[0] = (float)ham->external_field_normal[0];
                normal[1] = (float)ham->external_field_normal[1];
                normal[2] = (float)ham->external_field_normal[2];
            }
            else
            {
                *magnitude = 0;
                normal[0] = 0;
                normal[1] = 0;
                normal[2] = 1;
            }
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Get_Anisotropy(State *state, float * magnitude, float * normal, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
            
            if (ham->anisotropy_indices.size() > 0)
            {
                // Magnitude
                *magnitude = (float)ham->anisotropy_magnitudes[0];

                // Normal
                normal[0] = (float)ham->anisotropy_normals[0][0];
                normal[1] = (float)ham->anisotropy_normals[0][1];
                normal[2] = (float)ham->anisotropy_normals[0][2];
            }
            else
            {
                *magnitude = 0;
                normal[0] = 0;
                normal[1] = 0;
                normal[2] = 1;
            }
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Get_Exchange_Shells(State *state, int * n_shells, float * jij, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;

        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );

        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();

            *n_shells = ham->exchange_shell_magnitudes.size();

            // Note the array needs to be correctly allocated beforehand!
            for (int i=0; i<ham->exchange_shell_magnitudes.size(); ++i)
            {
                jij[i] = (float)ham->exchange_shell_magnitudes[i];
            }
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

int  Hamiltonian_Get_Exchange_N_Pairs(State *state, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
            image->hamiltonian->Name() + " Hamiltonian: fetching exchange pairs is not yet implemented...", idx_image, idx_chain );
        return 0;
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
        return 0;
    }
}

void Hamiltonian_Get_Exchange_Pairs(State *state, float * idx[2], float * translations[3], float * Jij, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
            image->hamiltonian->Name() + " Hamiltonian: fetching exchange pairs is not yet implemented...", idx_image, idx_chain );
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

void Hamiltonian_Get_DMI_Shells(State *state, int * n_shells, float * dij, int * chirality, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();
            
            *n_shells  = ham->dmi_shell_magnitudes.size();
            *chirality = ham->dmi_shell_chirality;
            
            for (int i=0; i<*n_shells; ++i)
            {
                dij[i] = (float)ham->dmi_shell_magnitudes[i];
            }
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}

int  Hamiltonian_Get_DMI_N_Pairs(State *state, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        Log( Utility::Log_Level::Warning, Utility::Log_Sender::API,
            image->hamiltonian->Name() + " Hamiltonian: fetching DMI pairs is not yet implemented...", idx_image, idx_chain );
        return 0;
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
        return 0;
    }
}

void Hamiltonian_Get_DDI(State *state, float * radius, int idx_image, int idx_chain) noexcept
{
    try
    {
        std::shared_ptr<Data::Spin_System> image;
        std::shared_ptr<Data::Spin_System_Chain> chain;
        
        // Fetch correct indices and pointers
        from_indices( state, idx_image, idx_chain, image, chain );
        
        if (image->hamiltonian->Name() == "Heisenberg")
        {
            auto ham = (Engine::Hamiltonian_Heisenberg*)image->hamiltonian.get();

            *radius = (float)ham->ddi_cutoff_radius;
        }
    }
    catch( ... )
    {
        spirit_handle_exception_api(idx_image, idx_chain);
    }
}