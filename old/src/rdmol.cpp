#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include <emscripten/emscripten.h>

#include "rdmol.h"

//standard libraries
#include <string>
#include <vector>
#include <utility>  // pair, get
#include <map>
#include <typeinfo>

// Boost include for rdkit dependence
#include <boost/cstdint.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>


/* RDkit libraries */
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/FileParsers/FileParsers.h>

// RDkit fingreprints
#include <DataStructs/ExplicitBitVect.h>
#include <GraphMol/Fingerprints/AtomPairs.h>
#include <GraphMol/Fingerprints/Fingerprints.h>
#include <GraphMol/Fingerprints/MorganFingerprints.h>
#include <GraphMol/Fingerprints/MACCS.h>
#include <DataStructs/BitOps.h>
#include <DataStructs/SparseIntVect.h>

// RDkit MolOps & Conformers
#include <GraphMol/MolOps.h>
#include <GraphMol/Conformer.h>


// RDkit stream Mol2File
#include <RDGeneral/StreamOps.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <RDGeneral/FileParseException.h>
#include <RDGeneral/BadFileException.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/FileParsers/MolFileStereochem.h>

// RDkit Drawing
//#include <GraphMol/MolDrawing/MolDrawing.h>
//#include <GraphMol/MolDrawing/DrawingToSVG.h>
#include <GraphMol/MolDraw2D/MolDraw2D.h>
#include <GraphMol/MolDraw2D/MolDraw2DSVG.h>

// RDkit canonical
#include <GraphMol/new_canon.h>

// RDkit 2D
#include <GraphMol/Depictor/RDDepictor.h>

// RDkit cpickle
#include <GraphMol/MolPickler.h>

// RDkit bond
#include <GraphMol/Bond.h>

// RDkit murko
#include <GraphMol/ChemTransforms/ChemTransforms.h>

// RDkit 3D
#include <GraphMol/DistGeomHelpers/Embedder.h>

// RDkit ForceField
// comments thegodone & Paolo => MMFF.h and Builder.h need to be patch to avoid class issues! 13_05_2015
#include <GraphMol/ForceFieldHelpers/MMFF/MMFF.h>
#include <GraphMol/ForceFieldHelpers/MMFF/Builder.h>
#include <GraphMol/ForceFieldHelpers/MMFF/AtomTyper.h>
#include <GraphMol/ForceFieldHelpers/UFF/UFF.h>
#include <ForceField/MMFF/Params.h>

// RDKit Writer
#include <GraphMol/FileParsers/MolWriters.h>

// RDkit subsearch
#include <GraphMol/RDKitQueries.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Substruct/SubstructUtils.h>


// RDkit AlignMols
#include <GraphMol/MolAlign/AlignMolecules.h>

// RDKit Charges
#include <GraphMol/PartialCharges/GasteigerCharges.h>
#include <GraphMol/PartialCharges/GasteigerParams.h>

// RDKit AutoCorr2d
#include <GraphMol/Descriptors/AUTOCORR2D.h>
// RDKit AutoCorr3D
//#include <GraphMol/Descriptors/AUTOCORR3D.h>



typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
//template<typename IndexType>
//typedef std::map<IndexType,int> StorageType;



using namespace std;
using RDKit::ROMol;
using RDKit::RWMol;



Molecule::Molecule(RWMol* mol): rdmol(mol) {}


/**
 * @brief [delete]
 * @details [action on rdmol during delete process]
 * @return [description]
 */
Molecule::~Molecule() {
  if(rdmol != 0) {
    //printf("Destroy called\n");
    delete rdmol;
    rdmol =0;
  }
}


/**
 * @brief [Similarity]
 * @details [generate a molecule from a smile]
 * 
 * @param  [compar one molecule to another molecule object]
 * @return [double]
 */
double Molecule::Similarity(const Molecule moltocompare, string similarityfunction,double a, double b) {

   RDKit::SparseIntVect< boost::uint32_t > * v1 =  RDKit::MorganFingerprints::getFingerprint(*rdmol,2);
   RDKit::SparseIntVect< boost::uint32_t > * v2 =  RDKit::MorganFingerprints::getFingerprint(moltocompare.getMol(),2);

   if (similarityfunction=="tanimoto")
   {  return TanimotoSimilarity (*v1,*v2);}


   if (similarityfunction=="dice")
      {  return DiceSimilarity (*v1,*v2);}

   if (similarityfunction=="tversky")
     {  return TverskySimilarity (*v1,*v2,a,b);}

   else return 0;
}

/**
 * @brief [fromSmile]
 * @details [generate a molecule from a smile]
 * 
 * @param smiles [string]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::smilesToMol(string smiles) {
  rdErrorLog->df_enabled = false;
  return new Molecule(RDKit::SmilesToMol(smiles));
}

/**
 * @brief [Mol2BlockToMol]
 * @details [generate a molecule from a Mol2Block structure]
 * 
 * @param molBlock [string of Mol2Block]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::Mol2BlockToMol(string molBlock) {
  rdErrorLog->df_enabled = false;
  return new Molecule(RDKit::Mol2BlockToMol(molBlock,true,false));
}

/**
 * @brief [MolBlockToMol]
 * @details [generate a moleculte from a MolBlock]
 * 
 * @param molBlock [string of MolBlock]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::MolBlockToMol(string molBlock) {
  rdErrorLog->df_enabled = false;
  return new Molecule(RDKit::MolBlockToMol(molBlock, true, false));
}


/**
 * @brief [smartsToMol]
 * @details [generate a molecule from a Smart]
 * 
 * @param smarts [string of smart]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::smartsToMol(string smarts) {
  rdErrorLog->df_enabled = false;
  return new Molecule(RDKit::SmartsToMol(smarts));
}

/**
 * @brief [molFromPickle]
 * @details [generate a molecule from a Pickle]
 * 
 * @param pickle [string of pickle]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::molFromPickle(string pickle) {
  RWMol *mol = new RWMol();
  RDKit::MolPickler::molFromPickle(pickle, *mol);
  return new Molecule(mol);
}


/**
 * @brief [MurckosmilesToMol]
 * @details [generate the murcko structure from a smile]
 * 
 * @param smi [string of smile]
 * @return [rdkit molecule object]
 */
Molecule* Molecule::MurckosmilesToMol(string smi)
{
     ROMol *mol=RDKit::SmilesToMol(smi);
     return new Molecule(dynamic_cast<RWMol *>(RDKit::MurckoDecompose(*mol)));
}

/**
 * @brief [newmolecule]
 * @details [generate a blank molecule container]
 * 
 * @return [rdkit blank molecule object]
 */
Molecule* Molecule::newmolecule()
{
     RWMol *mol= new RWMol();
     return new Molecule(mol);
}

/**
 * @brief [addAtom]
 * @details [adding atom to a molecule using a atomid]
 * 
 * @param atomid [integer starting at 0]
 * @return [define rdkit molecule atom]
 */
unsigned int Molecule::addAtom (int atomid)
{
   RDKit::Atom* atom= new RDKit::Atom(atomid);  
   return rdmol->addAtom(atom);
}       


/**
 * @brief [addBond]
 * @details [generate bond between atoms]
 * 
 * @param beginAtomIdx [unsigned int begin atom index of the bond]
 * @param endAtomIdx [unsigned int end atom index of the bond]
 * @param bondtypeid [int bondtype of the bond]
 * @return [set a bond between two atoms in rdkit molecule object]
 */

//// need to enumerate the BondType & BondDir lists ... for emscripten ???
unsigned int Molecule::addBond (unsigned int beginAtomIdx, unsigned int endAtomIdx, int bondtypeid)
{  
   // RDKit::Bond::BondType realbondtype = (RDKit::Bond::BondType)enum.Parse(typeof(RDKit::Bond::BondType), bondtype);
   RDKit::Bond::BondType castEnum = (RDKit::Bond::BondType)bondtypeid;
   return rdmol->addBond(beginAtomIdx,endAtomIdx,castEnum);
}       


/**
 * @brief [setBondDir]
 * @details [set the bond direction]
 * 
 * @param Bondid [int bond index value]
 * @param bonddirid [int bond dircection index]
 */
void Molecule::setBondDir (int Bondid, int bonddirid)
{
  RDKit::Bond::BondDir castEnum = (RDKit::Bond::BondDir)bonddirid;
  rdmol->getBondWithIdx(Bondid)->setBondDir(castEnum);
}       


/**
 * @brief [CanonicalRankAtoms]
 * @details [canonical ordering label of atoms]
 * 
 * @param breakTies [bool default true]
 * @param includeChirality [boll default true]
 * @param includeIsotopes [bool default true]
 */
vector<unsigned int> Molecule::CanonicalRankAtoms(bool breakTies=true, bool includeChirality=true, bool includeIsotopes=true){
    std::vector<unsigned int> ranks(rdmol->getNumAtoms());
    RDKit::Canon::rankMolAtoms(*rdmol, ranks, breakTies, includeChirality, includeIsotopes);
    return ranks;
  }




vector<double> Molecule::getAdjacencyMatrix(bool useBO) {  
      int nAts = rdmol->getNumAtoms();
      vector<double> res(nAts*nAts);
      for(RWMol::BondIterator bondIt=rdmol->beginBonds();
          bondIt!=rdmol->endBonds();bondIt++){
        
        if(!useBO){
          int beg=(*bondIt)->getBeginAtomIdx();
          int end=(*bondIt)->getEndAtomIdx();
          res[beg*nAts+end] = 1;
          res[end*nAts+beg] = 1;
        }
        else {
          int begIdx=(*bondIt)->getBeginAtomIdx();
          int endIdx=(*bondIt)->getEndAtomIdx();
          RDKit::Atom  *beg=rdmol->getAtomWithIdx(begIdx);
          RDKit::Atom  *end=rdmol->getAtomWithIdx(endIdx);
          res[begIdx*nAts+endIdx] = (*bondIt)->getValenceContrib(beg);
          res[endIdx*nAts+begIdx] = (*bondIt)->getValenceContrib(end);
        }
      }
      return res;
}




// compute the bfs to store the spheres & distance
/*

 function bfs (graph, startNode, len) {
      var parents = [];
      var sphereid = [];
      var spheres = [];
      var bondtype = [];
      var queue = [];
      var visited = [];
      var current;
      queue.push(startNode);
      parents[startNode] = null;
      bondtype[startNode] = null;
      sphereid[startNode]=0;
      visited[startNode] = true;

      while (queue.length) {
        current = queue.shift();
        for (var i = 0; i < graph.length; i += 1) {
          if (i !== current && graph[current][i]>0 && !visited[i]) {  // add >0 to include the bond weight instead of boolean matrix!
            parents[i] = current;
            sphereid[i] = sphereid[current]+1;
            bondtype[i] = graph[current][i];
            visited[i] = true;
            queue.push(i);
          }
        
        }
      }
    // extract the resulting spheres atomindexes based on the distance to the core! 
    for (var j=1;j<len;j++) {
        var spherescore = [];

        for (var i =0;i<graph.length;i+=1){
          if(sphereid[i]===j) {
            spherescore.push(i); 
          }
      }
    spheres.push(spherescore);

    }
    return {spheres,parents,bondtype};
}


Molecule::BFS(int StartIndex)
{
    RDKit::BitVect _notVisited;
    std::queue<RDKit::Atom *> _queue;
    std::vector<int> _depth;
    RDKit::Atom _ptr = rdmol->GetAtom(StartIndex);

   
    _notVisited.Resize(rdmol->NumAtoms());
    _notVisited.SetRangeOn(0, rdmol->NumAtoms() - 1);

    if (!_ptr) return;
    _notVisited.SetBitOff(_ptr->GetIdx() - 1);

    // Set up storage for the depths
    _depth.resize(rdmol->NumAtoms() + 1, 0);
    _depth[_ptr->GetIdx()] = 1;

    vector<RDKit::Bond*>::iterator i;
    RDKit::Atom *a;

    // look at the javascript version!

    for (a = _ptr->BeginNbrAtom(i); a; a = _ptr->NextNbrAtom(i))
      {
        _queue.push(a);
        _depth[a->GetIdx()] = 2;
        _notVisited.SetBitOff(a->GetIdx() - 1);
      }


    // queue and depth
    return _queue;
  }

*/



unsigned int Molecule::enumNbrConjGrp( std::vector<unsigned int> &atomConjGrpIdx,  std::vector<unsigned int> &bondConjGrpIdx, std::map<unsigned int, std::vector<unsigned int> > &m)
{
  unsigned int nConjGrp = 0;
  unsigned int nb = rdmol->getNumBonds();
  bondConjGrpIdx.resize(nb, nb);
  unsigned int na = rdmol->getNumAtoms();
  atomConjGrpIdx.resize(na, na);
  for (unsigned int i = 0; i < nb; ++i) {
    const RDKit::Bond *bi = rdmol->getBondWithIdx(i);
    unsigned int biBeginIdx = bi->getBeginAtomIdx();
    unsigned int biEndIdx = bi->getEndAtomIdx();
    if (bi->getIsConjugated() && (bondConjGrpIdx[i] == nb)) {
      // assign this conjugate bond to the matching group, if any
      for (unsigned int j = 0;
        (bondConjGrpIdx[i] == nb) && (j < nb); ++j) {
        if ((i == j) || (bondConjGrpIdx[j] == nb))
          continue;
        const RDKit::Bond *bj = rdmol->getBondWithIdx(j);
        if ((bj->getBeginAtomIdx() == biBeginIdx) || (bj->getBeginAtomIdx() == biEndIdx) || (bj->getEndAtomIdx() == biBeginIdx)   || (bj->getEndAtomIdx() == biEndIdx))
          bondConjGrpIdx[i] = bondConjGrpIdx[j];
      }
      // no existing group matches: create a new group
      if (bondConjGrpIdx[i] == nb)
        bondConjGrpIdx[i] = nConjGrp++;
    }
  }
  for (unsigned int i = 0; i < nb; ++i) {
    if (bondConjGrpIdx[i] != nb) {
      const RDKit::Bond *bi = rdmol->getBondWithIdx(i);
      unsigned int biBeginIdx = bi->getBeginAtomIdx();
      unsigned int biEndIdx = bi->getEndAtomIdx();
      atomConjGrpIdx[biBeginIdx] = bondConjGrpIdx[i];
      atomConjGrpIdx[biEndIdx] = bondConjGrpIdx[i];
    }
  }
  for (unsigned int ai = 0; ai < na; ++ai) {
    if (atomConjGrpIdx[ai] == na)
      continue;
    ROMol::ADJ_ITER nbrIdx, endNbrs;
    boost::tie(nbrIdx, endNbrs) =
      rdmol->getAtomNeighbors(rdmol->getAtomWithIdx(ai));
    std::vector <unsigned int> v;
    for (; nbrIdx != endNbrs; ++nbrIdx) {
      const RDKit::Atom *atom = rdmol->getAtomWithIdx(*nbrIdx);
      if (atom->getAtomicNum() == 1)
        continue;
      unsigned int aiNbr = atom->getIdx();
      if (atomConjGrpIdx[aiNbr] == na) {
        if (m.find(aiNbr) == m.end())
          m[aiNbr] = v;
        if (std::find(m[aiNbr].begin(), m[aiNbr].end(),
          atomConjGrpIdx[ai]) == m[aiNbr].end())
          m[aiNbr].push_back(atomConjGrpIdx[ai]);
      }
    }
  }
  return nConjGrp;
}


unsigned int Molecule::picontacts() {
  std::vector<unsigned int> atomConjGrpIdx;
  std::vector<unsigned int> bondConjGrpIdx;
  std::map<unsigned int, std::vector<unsigned int> > m;
  enumNbrConjGrp(atomConjGrpIdx, bondConjGrpIdx, m);
  for (std::map<unsigned int, std::vector<unsigned int> >::const_iterator
    mit = m.begin(); mit != m.end(); ++mit) {
    std::cout << mit->first << "\t";
    for (std::vector<unsigned int>::const_iterator
      vit = mit->second.begin(); vit != mit->second.end(); ++vit)
      std::cout << *vit << (vit == (mit->second.end() - 1) ? "\n" : ",");
  }
return 0;
}




unsigned int Molecule::getAtomCountRingBonds(int atomid)
{
  RDKit::Atom *atom = rdmol->getAtomWithIdx(atomid);
  unsigned int c = 0;
  const ROMol mol = *rdmol;
  RDKit::RingInfo *ringInfo = mol.getRingInfo();
  ROMol::OEDGE_ITER nbrIdx, endNbrs;
  boost::tie(nbrIdx, endNbrs) = mol.getAtomBonds(atom);
  for (; nbrIdx != endNbrs; ++nbrIdx) {
    unsigned int nbi = mol[*nbrIdx].get()->getIdx();
    if (ringInfo->numBondRings(nbi))
      ++c;
  }
  return c;
}

unsigned int Molecule::getAtomHeavyValence(int atomid)
{
  RDKit::Atom *atom = rdmol->getAtomWithIdx(atomid);
  unsigned int hv = 0;
  const ROMol mol = *rdmol;
  ROMol::ADJ_ITER nbrIdx, endNbrs;
  boost::tie(nbrIdx, endNbrs) = mol.getAtomNeighbors(atom);
  for (; nbrIdx != endNbrs; ++nbrIdx) {
    const RDKit::Atom *atomNbr = mol[*nbrIdx].get();
    if (atomNbr->getAtomicNum() != 1)
      ++hv;
  }
  return hv;
}

unsigned int Molecule::getAtomHeteroValence(int atomid)
{
  RDKit::Atom *atom = rdmol->getAtomWithIdx(atomid);
  unsigned int hv = 0;
  const ROMol mol = *rdmol;
  ROMol::ADJ_ITER nbrIdx, endNbrs;
  boost::tie(nbrIdx, endNbrs) = mol.getAtomNeighbors(atom);
  for (; nbrIdx != endNbrs; ++nbrIdx) {
    const RDKit::Atom *atomNbr = mol[*nbrIdx].get();
    if ((atomNbr->getAtomicNum() != 1)
      && (atomNbr->getAtomicNum() != 6))
      ++hv;
  }
  return hv;
}

unsigned int Molecule::getAtomValence(int atomid)
{
  RDKit::Atom *atom = rdmol->getAtomWithIdx(atomid);
  return atom->getExplicitValence();
}




// not working... enumeration is wrong
vector<int> Molecule::getAtomNeighbors(int atomid) {
    vector<int> ANids;

    RDKit::ROMol::ADJ_ITER ai1,ai2;
    boost::tie(ai1,ai2) = rdmol->getAtomNeighbors(rdmol->getAtomWithIdx(atomid));
    while(ai1!=ai2){
      //cout << to_string((*rdmol->getAtomWithIdx(*ai1)).getIdx()) << endl;
      ANids.push_back((*rdmol->getAtomWithIdx(*ai1)).getIdx());
      ai1++;
    }
    return ANids;
}

// not working... enumeration is wrong
vector<double> Molecule::getBondNeighbors(int atomid) {
        vector<double> BNtype;
        double btd;
        int nAtomIdx;
        RDKit::ROMol::ADJ_ITER ai1,ai2;
        boost::tie(ai1,ai2) = rdmol->getAtomNeighbors(rdmol->getAtomWithIdx(atomid));
        while(ai1!=ai2){
          nAtomIdx =(*rdmol->getAtomWithIdx(*ai1)).getIdx();
          btd = (*rdmol->getBondBetweenAtoms(atomid, nAtomIdx)).getBondTypeAsDouble();
          BNtype.push_back(btd);
          ai1++;
        }
        return BNtype;
}







/**
 * @brief [TanimotoSimilarityfromSmile ]
 * @details [compute tanimoto similarity of molecule vs a given smile]
 * 
 * @param smilesref [smile string of the molecule to compare]
 * @return [return the tanimoto similarity]
 */
double  Molecule::TanimotoSimilarityfromSmile ( string smilesref)       
{
   ROMol *mol=RDKit::SmilesToMol(smilesref);
   RDKit::SparseIntVect< boost::uint32_t > * v1 =  RDKit::MorganFingerprints::getFingerprint(*rdmol,2);
   RDKit::SparseIntVect< boost::uint32_t > * v2 =  RDKit::MorganFingerprints::getFingerprint(*mol,2);

   return TanimotoSimilarity (*v1,*v2);
}


/**
 * @brief [DiceSimilarityfromSmile]
 * @details [compute Dice similarity of molecule vs a given smile]
 * 
 * @param smilesref [smile string of the molecule to compare]
 * @return [return the Dice similarity]
 */
double  Molecule::DiceSimilarityfromSmile ( string smilesref)       
{

   ROMol *mol=RDKit::SmilesToMol(smilesref);
   RDKit::SparseIntVect< boost::uint32_t > * v1 =  RDKit::MorganFingerprints::getFingerprint(*rdmol,2);
   RDKit::SparseIntVect< boost::uint32_t > * v2 =  RDKit::MorganFingerprints::getFingerprint(*mol,2);

   return DiceSimilarity (*v1,*v2);
}

/**
 * @brief [TverskySimilarityfromSmile]
 * @details [compute Tversky similarity of molecule vs a given smile]
 * 
 * @param smilesref [smile string of the molecule to compare]
 * @param a [a parameter of Tversky similarity]
 * @param b [b parameter of Tversky similarity]
 * @return [return the Tversky similarity]
 */
double  Molecule::TverskySimilarityfromSmile ( string smilesref, double a, double b)       
{
   ROMol *mol=RDKit::SmilesToMol(smilesref);
   RDKit::SparseIntVect< boost::uint32_t > * v1 =  RDKit::MorganFingerprints::getFingerprint(*rdmol,2);
   RDKit::SparseIntVect< boost::uint32_t > * v2 =  RDKit::MorganFingerprints::getFingerprint(*mol,2);

   return TverskySimilarity (*v1,*v2,a,b);
}




/**
 * @brief [getNumAtoms]
 * @details [get the number of atoms of the molecule]
 * @return [number of atoms]
 */
unsigned int Molecule::getNumAtoms()
{
    return rdmol->getNumAtoms();
}


/**
 * @brief [getAtomCode]
 * @details [get the atom code (atompair)]
 * 
 * @param atomid [int index of the atom]
 * @return [uint32 atom code]
 */
boost::uint32_t Molecule::getAtomCode(int atomid) {
  return RDKit::AtomPairs::getAtomCode(rdmol->getAtomWithIdx(atomid));
}


/**
 * @brief [getAtomPairFingerprint]
 * @details [compute the list (not the map) of Atompair code + Atompair occurence]
 * @return [vector of int first portion of the list contains atompair code and second portion the atompair occurence]
 */
vector<int> Molecule::getAtomPairFingerprint() {
   RDKit::SparseIntVect<int> * finger =  RDKit::AtomPairs::getAtomPairFingerprint(*rdmol);
   RDKit::SparseIntVect<int>::StorageType gnze = finger->getNonzeroElements();

    int elementsize=gnze.size();
    vector<int>  result(2*elementsize);
    
    map<int, int>::iterator it;
    
    int idx = 0;
    for (it = gnze.begin(); it != gnze.end(); it++ )
    {
        result[idx]=it->first;
        result[elementsize+idx]=it->second;
        idx=idx+1;
    }
    return result;
}


/**
 * @brief [getHashedAtomPairFingerprint]
 * @details [compute Hashed list (not the map) of Atompair code + Atompair occurence]
 * 
 * @param size [length of the distance to the core atom to be include in the AtomPait Fingerprint]
 * @param atomid1 [from atom index 1]
 * @param atomid2 [to atom index 2]
 * @return [vector of int first portion of the list contains atompair code and second portion the atompair occurence]
 */
vector<int> Molecule::getHashedAtomPairFingerprint(int size, int atomid1, int atomid2) {
  RDKit::SparseIntVect<int> * finger =  RDKit::AtomPairs::getHashedAtomPairFingerprint(*rdmol,size,atomid1, atomid2);
  RDKit::SparseIntVect<int>::StorageType gnze = finger->getNonzeroElements();
  
  int elementsize=gnze.size();
    vector<int>  result(2*elementsize);
    
    map<int, int>::iterator it;
    
    int idx = 0;
    for (it = gnze.begin(); it != gnze.end(); it++ )
    {
        result[idx]=it->first;
        result[elementsize+idx]=it->second;
        idx=idx+1;
    }
    return result;
}

/**
 * @brief [getHashedAtomPairFingerprintAsBitVect]
 * @details [compute Hashed Atompair BitVect of the core atom]
 * 
 * @param size [length of the distance to the core atom to be include in the AtomPait Fingerprint]
 * @param atomid1 [from atom index 1]
 * @param atomid2 [to atom index 2]
 * @return [string of BitVector]
 */
string Molecule::getHashedAtomPairFingerprintAsBitVect(int size, int atomid1, int atomid2) {
  ExplicitBitVect * finger = RDKit::AtomPairs::getHashedAtomPairFingerprintAsBitVect(*rdmol,size,atomid1, atomid2);
  return BitVectToText(*finger);
}


/**
 * @brief [pickleMol]
 * @details [convert molecule to binary code from storage of pickle]
 * @return [string pickle of the molecule]
 */
string Molecule::pickleMol()
{
    string res;
    RDKit::MolPickler::pickleMol(*rdmol,res);
    return res;
}


/**
 * @brief [getRDKFingerprintMol]
 * @details [get RDKit fingerprint string]
 * 
 * @return [string of fingerprint]
 */
string Molecule::getRDKFingerprintMol()
{
    ExplicitBitVect* finger =  RDKit::RDKFingerprintMol(*rdmol);
    return BitVectToText(*finger);
}

/**
 * @brief [getLayeredFingerprintMol]
 * @details [get Layered fingerprint string]
 * 
 * @return [string of fingerprint]
 */
string Molecule::getLayeredFingerprintMol(unsigned int layer,unsigned int sizes,unsigned int lengths)
{
    ExplicitBitVect* finger =  RDKit::LayeredFingerprintMol(*rdmol,layer, sizes,lengths);
    return BitVectToText(*finger);
}


/**
 * @brief [getMACCSFingerprints]
 * @details [get MACCS fingerprint string]
 * 
 * @return [string of fingerprint]
 */
string Molecule::getMACCSFingerprints()
{
    ExplicitBitVect* finger =  RDKit::MACCSFingerprints::getFingerprintAsBitVect(*rdmol);
    return BitVectToText(*finger);
}


/**
 * @brief [getPatternFingerprintMol]
 * @details [get Pattern fingerprint string]
 * 
 * @return [string of fingerprint]
 */
string Molecule::getPatternFingerprintMol()
{
    ExplicitBitVect* finger =  RDKit::PatternFingerprintMol(*rdmol);
    return BitVectToText(*finger);
}


/**
 * @brief [getMorganFingerprints]
 * @details [get Morgan fingerprint string]
 * 
 * @return [string of fingerprint]
 */
string Molecule::getMorganFingerprints(unsigned int sizes,unsigned int lengths)
{
    ExplicitBitVect* finger =  RDKit::MorganFingerprints::getFingerprintAsBitVect(*rdmol,sizes, lengths);
    return BitVectToText(*finger);
}


/**
 * @brief [getMorganUFP]
 * @details [get Morgan fingerprint string]
 * @param sizes [length of the vector to be generated]

 * @return [key getOnBit keys like getNonzeroElements().key()]
 */
vector<boost::uint32_t> Molecule::getMorganFingerprintsKeys(unsigned int sizes)
{   int nbatoms=rdmol->getNumAtoms();
    std::vector<boost::uint32_t> *invars=0;
    bool useChirality=false;
    bool useBondTypes=true;
    bool useFeatures=false;
    bool useCounts=true;
    std::vector<boost::uint32_t> *froms=0;

    RDKit::SparseIntVect<boost::uint32_t> * fp = RDKit::MorganFingerprints::getFingerprint(*rdmol,sizes,invars,froms,useChirality,useBondTypes,useCounts,false);
    RDKit::SparseIntVect<boost::uint32_t>::StorageType gnze = fp->getNonzeroElements();

    int elementsize=gnze.size();
    vector<boost::uint32_t>  result(elementsize);
    
    map<boost::uint32_t, int>::iterator it;
    
    int idx = 0;
    for (it = gnze.begin(); it != gnze.end(); it++ )
    {
        result[idx]=it->first;
        idx=idx+1;
    }
    return result;

}


/**
 * @brief [getMorganFingerprints_getOnBbits]
 * @details [get Morgan fingerprint index vector of Bits]
 * 
 * @param sizes [length of the vector to be generated]
 * @param lengths [Morgan FP value]
 * @return [Morgan fingerprint index vector of Bits]
 */
vector<int> Molecule::getMorganFingerprints_getOnBbits(unsigned int sizes,unsigned int lengths)
{
    ExplicitBitVect* finger =  RDKit::MorganFingerprints::getFingerprintAsBitVect(*rdmol,sizes, lengths);
    IntVect onBits;
    finger->getOnBits(onBits);
    return onBits;
}


/**
 * @brief [getMorganFingerprints_getNonzeroElements]
 * @details [get Morgan fingerprint map of True Bits]
 * 
 * @param sizes [length of the vector to be generated]
 * @return [Morgan fingerprint map vector of True Bits]
 */
map<boost::uint32_t, int> Molecule::getMorganFingerprints_getNonzeroElements(unsigned int sizes)
{
    RDKit::SparseIntVect<boost::uint32_t> * fp =  RDKit::MorganFingerprints::getFingerprint(*rdmol,sizes);
    RDKit::SparseIntVect<boost::uint32_t>::StorageType gnze = fp->getNonzeroElements();
    //map<boost::uint32_t,int> gnze ;
    
    /*for(RDKit::SparseIntVect<boost::uint32_t>::StorageType::const_iterator iter=nze.begin();
        iter!=nze.end();++iter){
        boost::uint32_t v = iter->first;
        int v2 = iter->second;
    }*/
    //gnze.insert(nze.begin(), nze.end());
    //std::cout << gnze << std::endl;

    /*
    map<boost::uint32_t, int>::iterator it;

    for ( it = gnze.begin(); it != gnze.end(); it++ )
    {
        std::cout << it->first  // string (key)
                  << ':'
                  << it->second   // string's value 
                  << std::endl ;
    }
    */
    return gnze;
}



/**
 * @brief [getMorganFingerprintslist]
 * @details [get Morgan fingerprint vector of True Bits & values unfolded]
 * 
 * @param sizes [length of the vector to be generated]
 * @return [Morgan fingerprint map vector of True Bits]
 */
vector<boost::uint32_t> Molecule::getMorganFingerprintslist(unsigned int sizes)
{
    RDKit::SparseIntVect<boost::uint32_t> * fp =  RDKit::MorganFingerprints::getFingerprint(*rdmol,sizes);
    RDKit::SparseIntVect<boost::uint32_t>::StorageType gnze = fp->getNonzeroElements();

    int elementsize=gnze.size();
    vector<boost::uint32_t>  result(2*elementsize);
    
    map<boost::uint32_t, int>::iterator it;
    
    int idx = 0;
    for (it = gnze.begin(); it != gnze.end(); it++ )
    {
        result[idx]=it->first;
        result[elementsize+idx]=it->second;
        idx=idx+1;
    }


    return result;
}


/**
 * @brief [findMolChiralCenters]
 * @details [find all molecule chiral centers]
 * 
 * @param includeUnassigned [boolean value option]
 * @return [return the list chiral centers]
 */
vector<string> Molecule::findMolChiralCenters(bool includeUnassigned)
{
  RDKit::MolOps::assignStereochemistry(*rdmol,false,true,true);
  int p = 0;
  const RDKit::Atom *atom;
  vector<string> centers;
    for (int i =0;i<rdmol->getNumAtoms();i++) { 
      atom = rdmol->getAtomWithIdx(i);
      std::cout << i  << std::endl; 
      vector<string>  plist=atom->getPropList();
      for (int x=0;x<plist.size();x++)
      {
      std::cout << plist[x]  << std::endl; 
      }
      if (atom->hasProp(RDKit::common_properties::_CIPCode)) {
         string cip;
         atom->getProp(RDKit::common_properties::_CIPCode, cip);
         std::cout << cip << std::endl; 

         centers[p] = i;
         centers[p+1] = cip;
         p=p+2;
      }
      else if (includeUnassigned and atom->hasProp(RDKit::common_properties::_ChiralityPossible)){
         centers[p] = i;
         centers[p+1] = '?';
         std::cout << "includeUnassigned true" << std::endl; 

      p=p+2;
    }
  }
  return centers;
}



/**
 * @brief [MMFFoptimizeMolecule]
 * @details [generate the 3D optimized molecule coordonate based on MMFF field force model]
 * 
 * @return [return the status of the optimization and the energie of the optimal structure]
 */
vector<double> Molecule::MMFFoptimizeMolecule()
{
    vector<double> res(2);
    pair<int, double> p = RDKit::MMFF::MMFFOptimizeMolecule(*rdmol);
    res[0] = static_cast<double>(p.first);
    res[1] = p.second;
    return res;
}


/**
 * @brief [MMFFoptimizeMolecule]
 * @details [generate the 3D optimized molecule coordonate based on MMFF field force model]
 * 
 * @param maxIters [number of max iteration option]
 * @param mmffVariant [MMFF variant field force option]
 * @return [return the status of the optimization and the energie of the optimal structure]
 */
vector<double> Molecule::MMFFoptimizeMolecule(int maxIters, string mmffVariant)
{
    pair<int, double> p = RDKit::MMFF::MMFFOptimizeMolecule(*rdmol,maxIters,mmffVariant);
    vector<double> res(2);
    res[0] = static_cast<double>(p.first);
    res[1] = p.second;
    return res;
}


/**
 * @brief [MMFFOptimizeMoleculeConfs]
 * @details [generate the 3D optimized molecule conformers coordonate based on MMFF field force model]
 * 
 * @param numThreads [number of Threads option ... not working in JS for the moment!]
 * @param maxIters [number of max iteration option]
 * @param mmffVariant [MMFF variant field force option]
 * @return [return the status of the optimization and the energie of the optimal structure]
 */

vector<double> Molecule::MMFFOptimizeMoleculeConfs (unsigned int numThreads,int  maxIters,string mmffVariant)
{  
   
   vector<pair< int, double>> p;
   RDKit::MMFF::MMFFOptimizeMoleculeConfs(*rdmol,p,numThreads,maxIters,mmffVariant);
   vector<double> res(2*p.size());
   

   for (int i = 0; i < p.size(); i++) {
            res[i*2] = static_cast<double>(p[i].first);
            res[i*2+1] = p[i].second;
    }
    return res;
}


/**
 * @brief [UFFoptimizeMolecule]
 * @details [generate the 3D optimized molecule coordonate based on Universal field force model]
 * 
 * @return [return the status of the optimization and the energie of the optimal structure]
 */
vector<double> Molecule::UFFOptimizeMolecule()
{
    vector<double> res(2);
    pair<int, double> p = RDKit::UFF::UFFOptimizeMolecule(*rdmol);
    res[0] = static_cast<double>(p.first);
    res[1] = p.second;
    return res;
}


/**
 * @brief [getproplist]
 * @details [get the list of properties computed for this molecule]
 * @return [vector of string]
 */
vector<string> Molecule::getproplist()
{
    return rdmol->getPropList();
}

/**
 * @brief [molToSmiles]
 * @details [convert molecule to smile string]
 * 
 * @return [smile string]
 */
string Molecule::molToSmiles()
{
    string smile =  RDKit::MolToSmiles(*rdmol);  
    return smile;

}

/**
 * @brief [molToMolfile]
 * @details [convert molecule to Molfile string]
 * 
 * @return [Molfile string]
 */
string Molecule::molToMolfile()
{
    stringstream ss;
    RDKit::SDWriter *writer = new RDKit::SDWriter(&ss);
    writer->write(*rdmol);
    writer->flush();
    delete writer;
    return ss.str();
}


/**
 * @brief [getPath]
 * @details [get current path in nodeJS]
 * @return [path of nodeJS script]
 */
string Molecule::getPath()
{
    char cCurrentPath[FILENAME_MAX];
    int size =sizeof(cCurrentPath);
    getcwd(cCurrentPath, size);
    string res = string(cCurrentPath);
    return res;

}
    
/**
 * @brief [sdwriteConfs]
 * @details [convert molecule conformer list into a unique Molfile string]
 * @return [Molfile String]
 */
string Molecule::sdwriteConfs()
{
    stringstream ss;
    RDKit::SDWriter *writer = new RDKit::SDWriter(&ss); // remove false argument!
    
    for(int i=0; i<rdmol->getNumConformers(); ++i){
         writer->write(*rdmol,i);
     }

    writer->flush();
    delete writer;
    return ss.str();
}


/**
 * @brief [compute2DCoords]
 * @details [generate 2D molecule coordinate matrix]
 * 
 * @return [int of boolean of the void]
 */
unsigned int Molecule::compute2DCoords()
{
    return RDDepict::compute2DCoords(*rdmol);
}


/**
 * @brief [getAtomsPos2D]
 * @details [get the Atoms 2D coordinate + sigma value of a SVG plot for similarity map processing]
 * @return [vector of float]
 */
vector<float> Molecule::getAtomsPos2D()
{
    double sigma = 0;
    int atomnumber = rdmol->getNumAtoms();
    vector<float>  res(2*atomnumber+1);

    RDDepict::compute2DCoords(*rdmol);
    WedgeMolBonds(*rdmol,&(rdmol->getConformer()));
    // adding the transparency option!
    RDKit::MolDrawOptions options;
    //options.clearBackground=true; not in 2015.3
    RDKit::MolDraw2DSVG drawer(300,300);
    drawer.drawMolecule(*rdmol);
    drawer.finishDrawing();

    // double count
    for (int i =0;i<2*atomnumber;i=i+2) { 
      RDGeom::Point2D atomcoords = drawer.getDrawCoords(i/2); // replace the getAtomsCoords by Draw
      res[i]=atomcoords[0]/300;  // rescale to 0-1 range!
      res[i+1]=atomcoords[1]/300;
    }
    

    if (rdmol->getNumBonds() >0){
      RDKit::Bond * bond = rdmol->getBondWithIdx(0);
      int idx1 = bond->getBeginAtomIdx();
      int idx2 = bond->getEndAtomIdx();
      sigma = 0.3*sqrt(pow(res[2*idx1]-res[2*idx2],2)+pow(res[2*idx1+1]-res[2*idx2+1],2));
    }

    else {
      sigma = 0.3*sqrt(pow(res[0]-res[2],2)+pow(res[1]-res[3],2));
    }
     // store sigma in the last position => odd list!
      res[2*atomnumber+1]=sigma;
    return res;


}


/**
 * @brief [get2DScale]
 * @details [return the scale on 2D image for plotting]
 * 
 * @return [double]
 */
double Molecule::get2DScale(vector<float> atcds, double width, double height) {
   double x_min =10;
   double y_min =10;
   double x_max =-10;
   double y_max =-10;
   for( int i = 0; i < atcds.size() -1 ; i=i+2 ) {
         if (atcds[i]<x_min ) x_min = atcds[i];
         if (atcds[i]>x_max ) x_max = atcds[i];
         if (atcds[i+1]<y_min ) y_min = atcds[i+1];
         if (atcds[i+1]>y_max ) y_max = atcds[i+1];
   }
   double x_range = x_max - x_min;
   double y_range = y_max - y_min;
   double scale;
   return scale = min( double( width ) / x_range , double( height ) / y_range );
}

/**
 * @brief [Drawing2D]
 * @details [generate the svg of a 2D molecule for plotting]
 * 
 * @return [string type svg]
 */
string Molecule::Drawing2D()
{
    RDDepict::compute2DCoords(*rdmol);
    WedgeMolBonds(*rdmol,&(rdmol->getConformer()));
    RDKit::MolDraw2DSVG drawer(300,300);
    drawer.drawMolecule(*rdmol);
    drawer.finishDrawing();
    return drawer.getDrawingText();
}



/**
 * @brief [EmbedMolecule]
 * @details [Generate a random 3D molecule coordinate matrix]
 * 
 * @param maxIterations [maximum iteration process]
 * @param seed [fix a seed for random predictible generation value]
 * 
 * @return [int value of the process]
 */
int Molecule::EmbedMolecule(unsigned int maxIterations, int seed)
{   
    return RDKit::DGeomHelpers::EmbedMolecule(*rdmol,maxIterations,seed);
}


/**
 * @brief [EmbedMolecule]
 * @details [Generate a random 3D molecule coordinate matrix]
 * 
 * @return [int value of the process]
 */
int Molecule::EmbedMolecule()
{   
    return RDKit::DGeomHelpers::EmbedMolecule(*rdmol);
}


/**
 * @brief [EmbedMultipleConfs]
 * @details [Generate conformers random 3D molecules coordinate matrixes]
 * 
 * @param numConfs [set number of conformers]
 * @param maxIterations [set max iteration]
 * @param seed [set the seed for random predictible generation value]
 * @return [int vector value of the process]
 */
vector<int> Molecule::EmbedMultipleConfs(unsigned int numConfs, unsigned int maxIterations, int seed)
{    
    return RDKit::DGeomHelpers::EmbedMultipleConfs(*rdmol,numConfs,maxIterations,seed);
}

/**
 * @brief [EmbedMultipleConfs]
 * @details [Generate conformers random 3D molecules coordinate matrixes]
 * @return [int vector value of the process]
 */
vector<int> Molecule::EmbedMultipleConfs()
{   
    return RDKit::DGeomHelpers::EmbedMultipleConfs(*rdmol);
}


/**
 * @brief [findSSSRnumber]
 * @details [count the number of rings in the molecule]
 * 
 * @return [int value of the process]
 */
int Molecule::findSSSRnumber()
{   vector<vector<int>> res ;
    return RDKit::MolOps::findSSSR(*rdmol,res);
}


/**
 * @brief [findSSSR]
 * @details [export rings lists into a map of "ringid, string" string containing atomid of the ring and comma separator]
 * 
 * @return [map of int, string]
 */
map<int, string> Molecule::findSSSR ()
{   vector<vector<int>> res ;
    int count = RDKit::MolOps::findSSSR(*rdmol,res);
    map<int, string> resmap;
    for (int it = 0; it < count; it++ )
    {   string s = "";
        for (int j=0; j<res[it].size(); j++) 
        {
          s=s+to_string(res[it][j]);
          if (j<res[it].size()-1) s=s+",";
        }
        resmap[it] = s;
    }
    return resmap;
}



/*
double Molecule::GetConformersRMS(int confId1, int confId2){
    RDKit::Conformer  conf1 = rdmol->getConformer(confId1);
    RDKit::Conformer  conf2 = rdmol->getConformer(confId2);
    //RDKit::MolAlign::alignMolConformers(*rdmol,{confId1,confId2});
    double ssr = 0 ;
    double d;
    for (int i =0;i<rdmol->getNumAtoms();i++) { 
      d = RDGeom::Point3D::Distance(conf1.getAtomPos(i),conf2.getAtomPos(i)) ;
      ssr += d*d ;
    }
    ssr = ssr/rdmol->getNumAtoms() ;
    return ssr;
}
*/

/**
 * @brief [AlignMol]
 * @details [AlignMol method]
 * 
 * @param smilesref [string]
 * @return [double]
 */
double Molecule::AlignMol(string smilesref){
    rdErrorLog->df_enabled = false;
    RWMol *prbMol = RDKit::SmilesToMol(smilesref);
    RDKit::MatchVectType matches;
    RDKit::SubstructMatch(*rdmol,*prbMol,matches,true);

    RDKit::MolOps::addHs(*prbMol);
    RDKit::MolOps::addHs(*rdmol);

    RDKit::DGeomHelpers::EmbedMolecule(*prbMol);
    RDKit::DGeomHelpers::EmbedMolecule(*rdmol);

    //pair<int, double> p = RDKit::MMFF::MMFFOptimizeMolecule(*rdquery);

    double res = RDKit::MolAlign::alignMol(*rdmol,*prbMol,-1,-1,&matches);
    
    // can return the rmds values need to look at that closely... in python not there
    return res;
}


/**
 * @brief [AlignMolConformers]
 * @details [AlignMol method using first conformer as reference of align]
 * 
 * @return [double]
 */
void Molecule::AlignMolConformers(){
    // can return the rmds values need to look at that closely... in python not there
    RDKit::MolAlign::alignMolConformers(*rdmol);

}



vector<double> Molecule::getConformersRMS(unsigned int confId1,unsigned int confId2, unsigned int maxIters){
    // can return the rmds values need to look at that closely... in python not there
    std::vector<double> RMSlist;
    std::vector<unsigned int> confId;
    confId.push_back(confId1);
    confId.push_back(confId2);
    // cannot pass * objects to align ... create an issue !
    RDKit::MolAlign::alignMolConformers(*rdmol,0,&confId,0,true,maxIters,&RMSlist);
    return RMSlist;

}




/**
 * @brief [addHs]
 * @details [addHs]
 * 
 * @param addHs [molecule with Hs]
 */
void Molecule::addHs()
{
    return RDKit::MolOps::addHs(*rdmol);
}

/**
 * @brief [removeHs]
 * @details [removeHs]
 * 
 * @param removeHs [molecule without Hs]
 */
void Molecule::removeHs()
{
    return RDKit::MolOps::removeHs(*rdmol);
}

/**
 * @brief [sanitizeMol]
 * @details [sanitized molecule]
 * 
 * @param sanitizeMol [molecule sanitized]
 */
void Molecule::sanitizeMol()
{
    return RDKit::MolOps::sanitizeMol(*rdmol);
}

/**
 * @brief [cleanUp]
 * @details [cleanUp]
 * 
 * @param cleanUp [molecule cleanUp]
 */
void Molecule::cleanUp()
{
    return RDKit::MolOps::cleanUp(*rdmol);
}

/**
 * @brief [Kekulize]
 * @details [Kekulize representation of the bond for plotting & descriptors...]
 * 
 * @param Kekulize [molecule Kekulized]
 */
void Molecule::Kekulize()
{
    return RDKit::MolOps::Kekulize(*rdmol);
}




/************************************** descriptors part ************************************/
/**
 * @brief [computeGasteigerCharges]
 * @details [compute the GasteigerCharges for eahc atoms of the molecule and store it as a atom property]
 */
void Molecule::computeGasteigerCharges()
{
    vector<double> charges(rdmol->getNumAtoms(),0);
    RDKit::computeGasteigerCharges(*rdmol, charges,12,false);
}


/**
 * @brief [getNumMacrocycles]
 * @details [count the number of macrocycles for SA score model]
 * @return [number of macrocycles]
 */
int Molecule::getNumMacrocycles()
{
    RDKit::VECT_INT_VECT rings = rdmol->getRingInfo()->atomRings();
    int macroCycleCount = 0;
    for (RDKit::VECT_INT_VECT_CI ringIter = rings.begin();ringIter != rings.end();ringIter++) {
        if (ringIter->size()>8) {
            macroCycleCount++;
        } 
    }
    return macroCycleCount;
}

/**
 * @brief [calcNumSpiroAtoms]
 * @details [count the number of Spiro Atoms for SA score model]
 * @return [number of spiro atoms]
 * caution must use RDKIT version >= 2016.03.01
 */
int Molecule::getNumSpiroAtoms()
{
  return  RDKit::Descriptors::calcNumSpiroAtoms (*rdmol);
}


/**
 * @brief [calcNumBridgeheadAtoms]
 * @details [count the number of Bridge head Atoms for SA score model]
 * @return [number of Bridge head atoms]
 * caution must use RDKIT version >= 2016.03.01
*/
int Molecule::getNumBridgeheadAtoms()
{
  return  RDKit::Descriptors::calcNumBridgeheadAtoms(*rdmol);
}



/**
 * @brief [getMW]
 * @details [get approximate MW int value]
 * 
 * @return [return approximate MW]
 */
int Molecule::getMW()
{
    return RDKit::Descriptors::calcAMW(*rdmol);
}

/**
 * @brief [ExactMW]
 * @details [get Exact MW]
 * 
 * @return [double Exact MW]
 */
double Molecule::ExactMW()
{
    return RDKit::Descriptors::calcExactMW(*rdmol);
}


/**
 * @brief [Formula]
 * @details [get molecular formula]
 * 
 * @return [string MF]
 */
string Molecule::Formula()
{
    return RDKit::Descriptors::calcMolFormula(*rdmol);
}

/**
 * @brief [Chi0v]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi0v]
 */
double Molecule::Chi0v()
{
    return   RDKit::Descriptors::calcChi0v(*rdmol);
}

/**
 * @brief [Chi1v]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi1v]
 */
double Molecule::Chi1v()
{
    return   RDKit::Descriptors::calcChi1v (*rdmol);
}

/**
 * @brief [Chi2v]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi2v]
 */
double Molecule::Chi2v()
{
    return   RDKit::Descriptors::calcChi2v (*rdmol);
}


/**
 * @brief [Chi3v]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi3v]
 */
double Molecule::Chi3v()
{
    return   RDKit::Descriptors::calcChi3v (*rdmol);
}


/**
 * @brief [Chi4v]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi4v]
 */
double Molecule::Chi4v()
{
    return   RDKit::Descriptors::calcChi4v (*rdmol);
}


/**
 * @brief [ChiNv]
 * @details [ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [ChiNv]
 */
double Molecule::ChiNv(unsigned int N)
{
    return   RDKit::Descriptors::calcChiNv (*rdmol,N);
}

/**
 * @brief [Chi0n]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi0n]
 */
double Molecule::Chi0n()
{
    return   RDKit::Descriptors::calcChi0n (*rdmol);
}

/**
 * @brief [Chi1n]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi1n]
 */
double Molecule::Chi1n()
{
    return   RDKit::Descriptors::calcChi1n (*rdmol);
}

/**
 * @brief [Chi2n]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi2n]
 */
double Molecule::Chi2n()
{
    return   RDKit::Descriptors::calcChi2n (*rdmol);
}

/**
 * @brief [Chi3n]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi3n]
 */
double Molecule::Chi3n()
{
    return   RDKit::Descriptors::calcChi3n (*rdmol);
}

/**
 * @brief [Chi4n]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Chi4n]
 */
double Molecule::Chi4n()
{
    return   RDKit::Descriptors::calcChi4n (*rdmol);
}



/**
 * @brief [ChiNn]
 * @details [nValence ConnectivityDescriptors list from  Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [ChiNn]
 */
double Molecule::ChiNn(unsigned int N)
{
    return   RDKit::Descriptors::calcChiNn(*rdmol,N);
}



/**
 * @brief [HallKierAlpha]
 * @details [calculate the Hall-Kier alpha value for a molecule From equation (58) of Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [HallKierAlpha]
 */
double Molecule::HallKierAlpha()
{
    return RDKit::Descriptors::calcHallKierAlpha (*rdmol);
}

/**
 * @brief [Kappa1]
 * @details [calculate the Hall-Kier alpha value for a molecule From equation (58) and (59) of Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Kappa1]
 */
double Molecule::Kappa1()
{
    return    RDKit::Descriptors::calcKappa1 (*rdmol);
}

/**
 * @brief [Kappa2]
 * @details [calculate the Hall-Kier alpha value for a molecule From equation (58) and (59) of Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Kappa2]
 */
double Molecule::Kappa2()
{
    return    RDKit::Descriptors::calcKappa2 (*rdmol);
}

/**
 * @brief [Kappa3]
 * @details [calculate the Hall-Kier alpha value for a molecule From equation (58) and (59) of Rev. Comp. Chem. vol 2, 367-422, (1991)]
 * 
 * @return [Kappa3]
 */
double Molecule::Kappa3()
{
    return    RDKit::Descriptors::calcKappa3 (*rdmol);
}

/**
 * @brief [logp_mr]
 * @details [Wildman-Crippen logp,mr values]
 * @return [array of logP and mr]
 */
vector<double> Molecule::logp_mr()
{    
    vector<double> res(2); 
    double logp, mr;
    RDKit::Descriptors::calcCrippenDescriptors (*rdmol,logp,mr);
    res[0] = logp;
    res[1] = mr;
    return res;
}

/**
 * @brief [getCrippenAtomContribs]
 * @details [Wildman-Crippen logp,mr Atom Contributions]
 * @return [array of atom contribution]
 */
vector<double> Molecule::getCrippenAtomContribs() {
    vector<double> logp(rdmol->getNumAtoms());
    vector<double> mr(rdmol->getNumAtoms());
    RDKit::Descriptors::getCrippenAtomContribs(*rdmol,logp,mr,true);
    vector<double> results;
    results.reserve(logp.size() + mr.size());
    results.insert(results.end(), logp.begin(), logp.end());
    results.insert(results.end(), mr.begin(), mr.end());
    return results;
}


/**
 * @brief [LipinskiHBA]
 * @details [number of Lipinski H-bond acceptors for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::LipinskiHBA()
{
    return RDKit::Descriptors::calcLipinskiHBA (*rdmol);
}

/**
 * @brief [LipinskiHBA]
 * @details [number of Lipinski H-bond donors for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::LipinskiHBD()
{
    return RDKit::Descriptors::calcLipinskiHBD (*rdmol);
}


/**
 * @brief [NumRotatableBonds]
 * @details [number of rotatable bonds for a molecule. does not count things like amide or ester bonds]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumRotatableBonds()
{
    return RDKit::Descriptors::calcNumRotatableBonds (*rdmol);
}

/**
 * @brief [NumHBA]
 * @details [number of H-bond donors for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumHBD()
{
    return RDKit::Descriptors::calcNumHBD (*rdmol);
}

/**
 * @brief [NumHBA]
 * @details [number of H-bond acceptors for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumHBA()
{
    return RDKit::Descriptors::calcNumHBA (*rdmol);
}


/**
 * @brief [NumHeteroatoms]
 * @details [number of heteroatoms for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumHeteroatoms()
{
    return RDKit::Descriptors::calcNumHeteroatoms (*rdmol);
}

/**
 * @brief [NumAmideBonds]
 * @details [number of amide bonds in a molecule]
 * 
 * @param calcNumAmideBonds [description]
 * @return [description]
 */
unsigned int Molecule::NumAmideBonds()
{
    return RDKit::Descriptors::calcNumAmideBonds (*rdmol);
}

/**
 * @brief [FractionCSP3]
 * @details [get fraction of C atoms that are SP3 hybridized]
 * 
 * @return [double]
 */
double Molecule::FractionCSP3()
{
    return RDKit::Descriptors::calcFractionCSP3 (*rdmol);
}


/**
 * @brief [NumRings]
 * @details [number of rings for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumRings()
{
    return RDKit::Descriptors::calcNumRings (*rdmol);
}

/**
 * @brief [NumAromaticRings]
 * @details [number of aromatic rings for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumAromaticRings()
{
    return   RDKit::Descriptors::calcNumAromaticRings (*rdmol);
}

/**
 * @brief [NumAliphaticRings]
 * @details [number of aliphatic (containing at least one non-aromatic bond) rings for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumAliphaticRings()
{
    return   RDKit::Descriptors::calcNumAliphaticRings (*rdmol);
}

/**
 * @brief [NumSaturatedRings]
 * @details [number of saturated rings for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumSaturatedRings()
{
    return  RDKit::Descriptors::calcNumSaturatedRings (*rdmol);
}

/**
 * @brief [NumHeterocycles]
 * @details [number of heterocycles for a molecule]
 * 
 * @return [unsigned int ]
 */
unsigned int Molecule::NumHeterocycles()
{
    return   RDKit::Descriptors::calcNumHeterocycles (*rdmol);
}

/**
 * @brief [NumAromaticHeterocycles]
 * @details [number of aromatic heterocycles for a molecule]
 * 
 * @return [unsigned int ]
 */
unsigned int Molecule::NumAromaticHeterocycles()
{
    return    RDKit::Descriptors::calcNumAromaticHeterocycles (*rdmol);
}

/**
 * @brief [NumAromaticCarbocycles]
 * @details [number of aromatic Carbocycles for a molecule]
 * 
 * @return [unsigned int ]
 */
unsigned int Molecule::NumAromaticCarbocycles()
{
    return RDKit::Descriptors::calcNumAromaticCarbocycles (*rdmol);
}

/**
 * @brief [NumSaturatedHeterocycles]
 * @details [number of saturated heterocycles for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumSaturatedHeterocycles()
{
    return   RDKit::Descriptors::calcNumSaturatedHeterocycles (*rdmol);
}


/**
 * @brief [NumSaturatedCarbocycles]
 * @details [number of saturated carbocycles for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumSaturatedCarbocycles()
{
    return RDKit::Descriptors::calcNumSaturatedCarbocycles (*rdmol);
}

/**
 * @brief [NumAliphaticHeterocycles]
 * @details [number of aliphatic (containing at least one non-aromatic bond) heterocycles for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumAliphaticHeterocycles()
{
    return RDKit::Descriptors::calcNumAliphaticHeterocycles (*rdmol);
}

/**
 * @brief [NumAliphaticCarbocycles]
 * @details [number of aliphatic (containing at least one non-aromatic bond) carbocycles for a molecule]
 * 
 * @return [unsigned int]
 */
unsigned int Molecule::NumAliphaticCarbocycles()
{
    return RDKit::Descriptors::calcNumAliphaticCarbocycles (*rdmol);
}


/**
 * @brief [LabuteASA]
 * @details [Labute ASA value for a molecule]
 * 
 * @return [double]
 */
double Molecule::LabuteASA()
{
    return RDKit::Descriptors::calcLabuteASA (*rdmol);
}


/**
 * @brief [getASAContribs]
 * @details [Labute ASA Atom Contributions]
 * @return [array of atom contribution]
 */
vector<double> Molecule::getASAContribs() {
    double hContrib=0.0;
    bool includeHs=true;
    vector<double> contribs(rdmol->getNumAtoms());
    RDKit::Descriptors::getLabuteAtomContribs(*rdmol,contribs,hContrib,includeHs,false);
    return contribs;
}


/**
 * @brief [TPSA]
 * @details [TPSA value for a molecule]
 * 
 * @return [double]
 */
double Molecule::TPSA()
{
    return RDKit::Descriptors::calcTPSA (*rdmol);
}

/**
 * @brief [getTPSAAtomContribs]
 * @details [TPSA Atom Contributions]
 * @return [array of atom contribution]
 */
vector<double>  Molecule::getTPSAAtomContribs() {
    vector<double> contribs(rdmol->getNumAtoms());
    RDKit::Descriptors::getTPSAAtomContribs(*rdmol,contribs,false);
    return contribs;
}


/**
 * @brief [getAutoCorr2D]
 * @details [Generate AUTOCORR2D vector]
 * @return [array of autoccorelation 2D]
 */
vector<double>  Molecule::getAutoCorr2D() {
    vector<double> contribs(192);
    RDKit::Descriptors::AUTOCORR2D(*rdmol,contribs);
    return contribs;
}


/**
 * @brief [getAutoCorr3D]
 * @details [Generate AUTOCORR3D vector]
 * @return [array of autoccorelation 3D]

vector<double>  Molecule::getAutoCorr3D() {
    vector<double> contribs(80);
    RDKit::Descriptors::AUTOCORR3D(*rdmol,contribs, -1);
    return contribs;
}
 */


/**
 * @brief [SlogP_VSA]
 * @details [SlogP_VSA]
 * 
 * @return [vector of dobule]
 */
vector< double > Molecule::SlogP_VSA()
{
    return RDKit::Descriptors::calcSlogP_VSA (*rdmol);
}

/**
 * @brief [SMR_VSA]
 * @details [SMR_VSA]
 * 
 * @return [vector of dobule]
 */
vector< double > Molecule::SMR_VSA() {
    return RDKit::Descriptors::calcSMR_VSA (*rdmol);
}

/**
 * @brief [PEO_VSA]
 * @details [PEO_VSA]
 * 
 * @return [vector of dobule]
 */
vector< double > Molecule::PEO_VSA()
{
    return RDKit::Descriptors::calcPEOE_VSA (*rdmol);
}

/**
 * @brief [MQNs]
 * @details [MQNs]
 * 
 * @return [vector of dobule]
 */
vector< unsigned int > Molecule::MQNs()
{
    return RDKit::Descriptors::calcMQNs (*rdmol);
}



/**
 * @brief [getSubstructMatches]
 * @details [determine the Substruct Matches from a smile reference numbers as vector of int (3 consecutive ints define a sub match group)]
 * 
 * @param smilesref [string]
 * @return [vector of int]
 */
vector<int> Molecule::getSubstructMatches(string smilesref)
{
    vector< RDKit::MatchVectType > matches;
    rdErrorLog->df_enabled = false;
    RWMol* rdquery = RDKit::SmartsToMol(smilesref);
    int matched = RDKit::SubstructMatch(*rdmol,*rdquery,matches,true);

    vector<int> vint;
    for(int idx=0;idx<matched;idx++){
        vector<pair<int, int> > invector= matches[idx];
        for (int idx2=0;idx2<invector.size();idx2++) {
             //std::cout <<  invector[idx2].second  << std::endl ;
             vint.push_back(invector[idx2].second);
        }
    }

    return vint;
}

/**
 * @brief [getSubstructMatchesNumber]
 * @details [determine the Substruct Matches from a smile reference total number]
 * 
 * @param smilesref [string]
 * @return [int]
 */
int Molecule::getSubstructMatchesNumber(string smilesref)
{
    vector< RDKit::MatchVectType > matches;
    rdErrorLog->df_enabled = false;
    RWMol* rdquery = RDKit::SmartsToMol(smilesref);
    int matched = RDKit::SubstructMatch(*rdmol,*rdquery,matches,true);
    
    return matched;
}

/**
 * @brief [HasSubstructMatchStr]
 * @details [determine if substructure (from smile) is in the current molecule]
 * 
 * @param smilesref [string]
 * @return [bool]
 */
bool Molecule::HasSubstructMatchStr(string smilesref)
{
    rdErrorLog->df_enabled = false;
    RWMol* rdquery = RDKit::SmartsToMol(smilesref);
    
    RDKit::MatchVectType res;
    return RDKit::SubstructMatch(*rdmol,*rdquery,res);
}



/**
 * @brief [getAtomicNums]
 * @details [determine the list of atomic numbers of the molecule]
 * 
 * @return [vector of int]
 */
vector<int> Molecule::getAtomicNums()
{
    vector<int> res;
    int atomnumber;
    int numAtoms =rdmol->getNumAtoms();
    for (int i =0;i<numAtoms;i++) {
        atomnumber = rdmol->getAtomWithIdx(i)->getAtomicNum();
        res.push_back(atomnumber);
    }
    return res;
}

vector<string> Molecule::getSymbols()
{
    vector<string> res;
    string atomsymbol;
    int numAtoms =rdmol->getNumAtoms();
    for (int i =0;i<numAtoms;i++) {
        atomsymbol = rdmol->getAtomWithIdx(i)->getSymbol();
        res.push_back(atomsymbol);
    }
    return res;
}




/******************************** get & set & has properties *******************************************/

/**
 * @brief [getProp]
 * @details [get a molecule Property value based of property name key]
 * 
 * @param key [string]
 * @return [string]
 */
string Molecule::getProp(string key) {
    string res;
    rdmol->getProp(key,res);
    return res;
}

/**
 * @brief [getAtomProp]
 * @details [get a atom Property value based of property name key]
 * 
 * @param key [string]
 * @return [string]
 */
string Molecule::getAtomProp(string key,int atomid) {
    string res;
    rdmol->getAtomWithIdx(atomid)->getProp(key,res);
    return res;
}


/**
 * @brief [getNumConformers]
 * @details [count number of conformers]
 * @return [int]
 */
int Molecule::getNumConformers() {
    return rdmol->getNumConformers();
}

/**
 * @brief [getConformer]
 * @details [select a conformers]
 * 
 * @param id [int]
 * @return [conformer]
 */
RDKit::Conformer Molecule::getConformer(int id) {
    return rdmol->getConformer(id);
}


/**
 * @brief [setProp]
 * @details [set a molecular property value for a property key]
 * 
 * @param key [string]
 * @param value [string]
 * 
 * @return [int]
 */
int Molecule::setProp(string key, string value) {
    rdmol->setProp(key,value);
    return 0;
}


/**
 * @brief [hasProp]
 * @details [determine if molecule has a key property]
 * 
 * @param key [string]
 * @return [boolean]
 */
bool Molecule::hasProp(string key) {
    return rdmol->hasProp(key);
}





/*
string Molecule::sdwritefile(string filename)
{


string smiString = "CN([CH](Cc3ccc(OS(c2cccc1c2ccnc1)(=O)=O)cc3)C(=O)N5CCN(c4ccccc4)CC5)S(c7cccc6c7ccnc6)(=O)=O \
                           C[n](n1)ccc1NC(=O)C(/C#N)=C/c2[s]cc(c2)Br O=C2C1CC3CC(C1)CC2C3 \
                           c8cc9cc3c5c(c(=O)cc4n1nc6c2c(ccc(c12)c(cc3)c45)cc7c6cc(=O)cc7)c9cc8 \
                           c1cccc(c1)CCCCNC(=O)C(/C#N)=C/c2ccc(O)c(c2)O \
                           COc(cc1)ccc1C(=C2)C=C(NC2=C)c3ccc(OC)cc3O \
                           CC(OC1C(CCCC3)C3C(CCCC2)C2C1OC(C)=O)=O \
                           Cl.C[N+](C)(C)CCO O=C2C1CC3CC(C1)CC2C3 \
                           CN3CCC25[CH]4CCC(=O)C5(C)Oc1c(O)ccc(c12)C[CH]34 \
                           c1ccccc1\\C=C/C=C\\Cl.c1ccccc1C=C(Cl)C1SCCC1.Cl\\C=C1/SCCC1 Cl\\C=C/Br \
                           c1ccccc1\\C=C/C=C\\C=C/Cl c1ccccc1C=C(Cl)C1SCCC1 Cl\\C=C1/SCCC1 Cl\\C=C/Br \
                           CN2C3CC(OC(=O)C(CO)c1ccccc1)CC2CC3 \
                           N2C3CC(OC(=O)C(CO)c1ccccc1)CC2CC3 \
                           C2C3CC(OC(=O)C(CO)c1ccccc1)CC2CC3 \
                           ClC=C1SCCC1 C/C=C/C(C1CCCCCC1)=O c1ccccc1\\C=C/C=C\\C=C/Cl \
                           C[n](n1)ccc1NC(=O)C(/C#N)=C/c2[s]cc(c2)Br \
                           C1CC(C(Cl)(Br)F)CC1 \
                           C1(C2)CCC2C=C1 \
                           OC2C1(C)CCCCC1CCC2 \
                           CC12CCCC3OC1CCCC23 \
                           CC(OC1C(CCCC3)C3C(CCCC2)C2C1OC(C)=O)=O \
                           ON=C(CC1=CC=CC=C1)[CH](C#N)C2=CC=CC=C2 \
                           COc(cc1)ccc1C(=C2/C#N)\\C=C(NC2=C(C#N)C#N)\\c3ccc(OC)cc3O \
                           COc(cc1)ccc1C(=C2)C=C(NC2=C)c3ccc(OC)cc3O";



    string ofile = filename;
    RDKit::SDWriter *sdfWriter = new RDKit::SDWriter(ofile);
    boost::char_separator<char> spaceSep(" ");
    tokenizer tokens(smiString,spaceSep);
    int j=0;
    for(tokenizer::iterator token=tokens.begin();token!=tokens.end(); ++token){
        ++j;
        std::string smi=*token;
        RWMol *m = RDKit::SmilesToMol(smi, 0, 1); 
        sdfWriter->write(*m);
        delete m;
    }

    sdfWriter->flush();
    delete sdfWriter;

    return to_string(j);
}



string Molecule::sdreadfile(string filename)

 {
    string ofile = filename;

  RDKit::SDMolSupplier reader(ofile);
  int i = 0;
  while (!reader.atEnd()) {
    ROMol *mol = reader.next();
    std::string mname;
    delete mol;
    i++;
  }

    return to_string(i);
}



void Molecule::save(string path, string data)
{
 std::string asm_code;
    asm_code += "localStorage.setItem( '";
    asm_code += path;
    asm_code += "', '";
    asm_code += data;
    asm_code += "' );";
    emscripten_run_script( asm_code.data() );

}


string Molecule::load(string path) 
{
    std::string asm_code;
    asm_code += "localStorage.getItem( '";
    asm_code += path;
    asm_code += "' );";
    std::string buffer = emscripten_run_script_string( asm_code.data() );
    return buffer;

}



int Molecule::readfile(string filename)

 {
  string line;

  ifstream myfile;
  myfile.open(filename);
  int j=0;
  if (myfile.is_open())
   {
     while ( getline (myfile,line)  && j<100)
     { 
       ++j;
       cout << line << '\n' ;
    }
    myfile.close();
  }

  else 
    std::cout << "Unable to open file" << std::endl; 

  return 1;


}




int Molecule::nodereadwritewithdata(string path, string data) {
    

    string asm_code;
    asm_code += "var fs = require('fs');";
    asm_code += "fs.writeFileSync('";
    asm_code += path;
    asm_code += "','";
    asm_code += data;
    asm_code += "');";
    emscripten_run_script( asm_code.data() );
    return 1;
}
*/

/*
int Molecule::nodereadwrite() {

  FILE *file;
  int res;
  char buffer[512];

  // write something locally with node
  EM_ASM(
    var fs = require('fs');
    fs.writeFileSync('foobar.txt', 'yeehaw');
  );

  // mount the current folder as a NODEFS instance
  // inside of emscripten
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
  );

  // read and validate the contents of the file
  file = fopen("/working/foobar.txt", "r");
  res = fread(buffer, sizeof(char), 6, file);
  fclose(file);

  // write out something new
  file = fopen("/working/foobar.txt", "w");
  res = fwrite("cheez", sizeof(char), 5, file);
  fclose(file);

  // validate the changes were persisted to the underlying fs
  EM_ASM(
    var fs = require('fs');
    var contents = fs.readFileSync('foobar.txt', { encoding: 'utf8' });

  );

  puts("success");

  return 0;
}









int Molecule::writefile(string filename, string data)

 {
    std::string asm_code;
    asm_code += "FS.writeFile('";
    asm_code += filename;
    asm_code += "', '";
    asm_code += data;
    asm_code += "' );";
    emscripten_run_script_string( asm_code.data() );


    std::ifstream file(filename);

    while(!file.eof() && !file.fail())
    {
        std::string line;
        getline(file, line);
        std::string name;
    
        std::cout << "read " << line << std::endl;

    }



  ofstream myfile;
  myfile.open (filename);
  myfile << data;
  myfile.close();
  

   return 1;
}



*/



