// LoadAdapters.h

#ifndef FLEXBAR_LOADADAPTERS_H
#define FLEXBAR_LOADADAPTERS_H


// Oligonucleotide sequences © 2018 Illumina, Inc.  All rights reserved.
// Obtained from https://support.illumina.com/bulletins/2016/12/what-sequences-do-i-use-for-adapter-trimming.html

namespace flexbar{
	
	Adapters TrueSeq_ltht = Adapters("TruSeq");
	TrueSeq_ltht.seq1 = "AGATCGGAAGAGCACACGTCTGAACTCCAGTCA";
	TrueSeq_ltht.seq2 = "AGATCGGAAGAGCGTCGTGTAGGGAAAGAGTGT";
	TrueSeq_ltht.info = "TruSeq LT and TruSeq HT-based kits";
	
	Adapters TrueSeq_methyl = Adapters("TrueSeq-Methyl");
	TrueSeq_methyl.seq1 = "AGATCGGAAGAGCACACGTCTGAAC";
	TrueSeq_methyl.seq2 = "AGATCGGAAGAGCGTCGTGTAGGGA";
	TrueSeq_methyl.info = "ScriptSeq and TruSeq DNA Methylation";
	
	Adapters TrueSeq_smallRNA = Adapters("TrueSeq-smallRNA");
	TrueSeq_smallRNA.seq1 = "TGGAATTCTCGGGTGCCAAGG";
	TrueSeq_smallRNA.info = "TruSeq Small RNA";
	
	Adapters TrueSeq_ribo = Adapters("TrueSeq-Ribo");
	TrueSeq_ribo.seq1 = "AGATCGGAAGAGCACACGTCT";
	TrueSeq_ribo.info = "TruSeq Ribo Profile";
	
	Adapters Nextera_TruSight = Adapters("Nextera-TruSight");
	Nextera_TruSight.seq1 = "CTGTCTCTTATACACATCT";
	Nextera_TruSight.seq2 = "CTGTCTCTTATACACATCT";
	Nextera_TruSight.info = "AmpliSeq, Nextera, Nextera DNA Flex, Nextera DNA, Nextera XT, Nextera Enrichment, Nextera Rapid Capture Enrichment, TruSight Enrichment, TruSight Rapid Capture Enrichment, TruSight HLA";
	
	Adapters Nextera_matepair = Adapters("Nextera-Matepair");
	Nextera_matepair.seq1 = "GATCGGAAGAGCACACGTCTGAACTCCAGTCAC";
	Nextera_matepair.seq2 = "GATCGGAAGAGCGTCGTGTAGGGAAAGAGTGT";
	Nextera_matepair.seqc = "CTGTCTCTTATACACATCT";
	Nextera_matepair.info = "Nextera Mate Pair";
}


template <typename TSeqStr, typename TString>
class LoadAdapters {

private:
	
	std::ostream *out;
	tbb::concurrent_vector<flexbar::TBar> adapters;
	
	const flexbar::RevCompMode m_rcMode;
	
public:
	
	LoadAdapters(const Options &o) :
		
		out(o.out),
		m_rcMode(o.rcMode){
	};
	
	
	virtual ~LoadAdapters(){};
	
	
	void loadSequences(const std::string filePath){
		
		using namespace std;
		using namespace flexbar;
		
		seqan::SeqFileIn seqFileIn;
		
		setFormat(seqFileIn, seqan::Fasta());
		
		if(! open(seqFileIn, filePath.c_str())){
			cerr << "\nERROR: Could not open file " << filePath << "\n" << endl;
			exit(1);
		}
		
		TSeqStrs seqs;
		TStrings ids;
		
		try{
			readRecords(ids, seqs, seqFileIn);
			
			map<TString, short> idMap;
			
			for(unsigned int i = 0; i < length(ids); ++i){
				
				if(idMap.count(ids[i]) == 1){
					cerr << "Two ";
					
					if(m_isAdapter) cerr << "adapters";
					else            cerr << "barcodes";
					
					cerr << " have the same name.\n";
					cerr << "Please use unique names and restart.\n" << endl;
					exit(1);
				}
				else idMap[ids[i]] = 1;
				
				if(! m_isAdapter || m_rcMode == RCOFF || m_rcMode == RCON){
					TBar bar;
					bar.id  =  ids[i];
					bar.seq = seqs[i];
					adapters.push_back(bar);
				}
				
				if(m_isAdapter && (m_rcMode == RCON || m_rcMode == RCONLY)){
					TString  id =  ids[i];
					TSeqStr seq = seqs[i];
					
					append(id, "_rc");
					seqan::reverseComplement(seq);
					
					TBar barRC;
					barRC.id        = id;
					barRC.seq       = seq;
					barRC.rcAdapter = true;
					adapters.push_back(barRC);
				}
			}
		}
		catch(seqan::Exception const &e){
			cerr << "\nERROR: " << e.what() << "\nProgram execution aborted.\n" << endl;
			close(seqFileIn);
			exit(1);
		}
		
		close(seqFileIn);
	};
	
	
	tbb::concurrent_vector<flexbar::TBar> getAdapters(){
		return adapters;
	}
	
	
	void setAdapters(tbb::concurrent_vector<flexbar::TBar> &newAdapters){
		adapters = newAdapters;
	}
	
	
	void printAdapters(std::string adapterName) const {
		
		using namespace std;
		
		const unsigned int maxSpaceLen = 23;
		
		stringstream s; s << adapterName;
		int len = s.str().length() + 1;
		
		if(len + 2 > maxSpaceLen) len = maxSpaceLen - 2;
		
		*out << adapterName << ":" << string(maxSpaceLen - len, ' ') << "Sequence:" << "\n";
		
		for(unsigned int i=0; i < adapters.size(); ++i){
			TString seqTag = adapters.at(i).id;
			
			int whiteSpaceLen = maxSpaceLen - length(seqTag);
			if(whiteSpaceLen < 2) whiteSpaceLen = 2;
			
			string whiteSpace = string(whiteSpaceLen, ' ');
			
			*out << seqTag << whiteSpace << adapters.at(i).seq << "\n";
		}
		*out << endl;
	}
	
};

#endif
