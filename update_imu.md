# Changes 
BNO Sensor does not need 1.8V - all circuitry removed.
    * Level shifting ciruitry removed
    * Regulator removed
        * 1.8V EN pin net removed

VIN Monitor divider is 2M and 1M
    * Changed to 200k and 100k temporarily

AP2112K VREG    
    * Changed pull-up to 100k from 330k

AP2112K VREG for Qwiic  
    * Changed 220K to 100K (R3)

Changed 220K to 100K (R29) on Artemis Boot
