
#ifndef __POLICY__
#define __POLICY__

// Quels sont les composants communs a une politique de cache.

// Base sur un input (Cache Stats) + Event (Cache Miss), identifie l'entree a ajouter
// (lookup container controller) et l entree a retirer.
// Interface avec le controller. 
// Policy doit pouvoir s

template< typename Stats_Value>
class Policy{

    public: 

    virtual void update_stats(){
        
    }


    private:



}

#endif