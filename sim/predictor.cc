#include "predictor.h"

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

#define bimodal false
#define global true
#define gshare false
#define correle false
#define local false
#define mixte false

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }

   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);

   #if global
     histsize = 10;
     histmask = (1 << histsize) - 1;
   #endif

   nentries = (1 << pcbits);        // nombre d'entrées dans la table
   pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
   countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
   historic = 0;
   table    = new uint32_t[nentries]();
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT64 PC)
{
  #if bimodal
    uint32_t v = table[PC & pcmask];
    return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
  #endif

  #if global
    uint32_t v = table[historic & histmask];
    return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
  #endif
}

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
  #if bimodal
    uint32_t v = table[PC & pcmask];
    if (((countmax + 1)/2) && (resolveDir == NOT_TAKEN)) {
      table[PC & pcmask] = 0;
    }
    else if (((countmax - 1)/2) && (resolveDir == TAKEN)) {
      table[PC & pcmask] = countmax;
    } else {
      table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
    }
  #endif

  #if global
    uint32_t v = table[historic & histmask];
    historic = (histmask) & ((historic << 1) | resolveDir) ;
    if (((countmax + 1)/2) && (resolveDir == NOT_TAKEN)) {
      table[historic & histmask] = 0;
    }
    else if (((countmax - 1)/2) && (resolveDir == TAKEN)) {
      table[historic & histmask] = countmax;
    } else {
      table[historic & histmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
    }
  #endif
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
