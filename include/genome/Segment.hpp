#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace vflib
{

class SequenceDistance
{
    public:
        virtual bool operator()(const std::string& a, const std::string& b) = 0;
};

class LevenshteinDistance : public SequenceDistance
{
    public:
        bool operator()(const std::string& a, const std::string& b)
        {
            size_t n = a.size()+1;
            size_t m = a.size()+1;

            size_t* d = new size_t[n * m];
            std::memset(d, 0, sizeof(size_t) * n * m);

            for (size_t i = 1, im = 0; i < m; ++i, ++im)
            {
                for (size_t j = 1, jn = 0; j < n; ++j, ++jn)
                {
                    if (a[jn] == b[im])
                    {
                        d[(i * n) + j] = d[((i - 1) * n) + (j - 1)];
                    }
                    else
                    {
                        d[(i * n) + j] = std::min(d[(i - 1) * n + j] + 1, /* A deletion. */
                                            std::min(d[i * n + (j - 1)] + 1, /* An insertion. */
                                                d[(i - 1) * n + (j - 1)] + 1)); /* A substitution. */
                    }
                }
            }

            size_t r = d[n * m - 1];

            delete [] d;

            return r;
        }
};

/*
* @brief Represent a genome string
* @details The class provides the all the possibile sequence structures: 
*  reverse, complement, reverse complemented
* @note
* Complement 
*    A -> T
*    T -> A
*    C -> G
*    G -> C
*/
class SequenceString
{
    private:
        std::vector<char> bases =  {'A','T','C','G'};
        std::unordered_map<char, char> complementBase =
            {{'A','T'}, {'T','A'}, {'C','G'}, {'G','C'}};
        std::string sequence;
        std::string reverse;
        std::string complement;
        std::string reverse_complement;

    public:
        inline std::string ComplementSequence(const std::string& sequence)
        {
            std::string complement(sequence);
            for(std::string::size_type i = 0; i < sequence.size(); ++i)
            {
                complement[i]=complementBase[sequence[i]];
            }
            return complement;
        }

        SequenceString(std::string sequence):
            sequence(sequence),
            reverse(sequence)
        {
            std::reverse(reverse.begin(), reverse.end());
            complement = ComplementSequence(sequence);
            reverse_complement = ComplementSequence(reverse);
            #ifdef VERBOSE
                std::cout << "Sequence: " << sequence << std::endl;
                std::cout << "Reverse: " << reverse << std::endl;
                std::cout << "Complement: " << complement << std::endl;
                std::cout << "Reverse Complement: " << reverse_complement << std::endl;
            #endif
        }

        SequenceString(const SequenceString& other):
            sequence(other.sequence),
            reverse(other.reverse),
            complement(other.complement),
            reverse_complement(other.reverse_complement){}

        SequenceString(const SequenceString&& other):
            sequence(std::move(other.sequence)),
            reverse(std::move(other.reverse)),
            complement(std::move(other.complement)),
            reverse_complement(std::move(other.reverse_complement)){}

        SequenceString():SequenceString(""){}

        /**
        * @note We consider as equivalent only sequences and their reverse complements. 
        *   In a double stranded molecule each strand is the reverse complement of each 
        *   other, so we must consider them as indistinguishable
        **/
        bool operator==(const SequenceString& other) const
        {
            return (sequence == other.sequence ||
                reverse_complement == other.sequence ||
                sequence == other.reverse_complement);
        }
        
        bool operator!=(const SequenceString& other) const
        {
            return *this != other;
        }

        SequenceString& operator=(const SequenceString& other)
        {
            this->sequence = other.sequence;
            this->reverse = other.reverse;
            this->complement = other.complement;
            this->reverse_complement = other.reverse_complement;
            return *this;
        }

        template<typename Distance=LevenshteinDistance>
        bool ComputeDistance(const SequenceString& other)
        {
            Distance distance;
            return distance(sequence, other.sequence);
        }

        
        inline std::string str() const {
            std::stringstream ss;
            ss << sequence;
            return ss.str();
        }

        inline const size_t GetLength() const {return sequence.size();}
        inline const std::string& Value() const {return sequence;}
        inline const std::string& Reverse() const {return reverse;}
        inline const std::string& Complement() const {return complement;}
        inline const std::string& ReverseComplement() const {return reverse_complement;}
};

class Segment
{
    private:
        std::string name;
        SequenceString sequence;
        uint32_t length;
        uint32_t reads_count;
        uint32_t fragment_count;
        uint32_t kmer_count;    
    
    public:
        Segment():
            name(""),  
            length(0),
            reads_count(0),
            fragment_count(0),
            kmer_count(0){}

        Segment(std::string name, 
            SequenceString sequence,
            uint32_t reads_count = 0,
            uint32_t fragment_count = 0,
            uint32_t kmer_count = 0):
            name(name), 
            sequence(std::move(sequence)), 
            length(sequence.GetLength()),
            reads_count(reads_count),
            fragment_count(fragment_count),
            kmer_count(kmer_count){}
        
        Segment(const Segment& other):
            name(other.name), 
            sequence(other.sequence), 
            length(other.length),
            reads_count(other.reads_count),
            fragment_count(other.fragment_count),
            kmer_count(other.kmer_count){}
        
        Segment(const Segment&& other):
            name(std::move(other.name)), 
            sequence(std::move(other.sequence)), 
            length(other.length),
            reads_count(other.reads_count),
            fragment_count(other.fragment_count),
            kmer_count(other.kmer_count){}
        
        bool operator==(const Segment& other) const
        {
            return sequence == other.sequence;
        }
        
        bool operator!=(const Segment& other) const
        {
            return sequence != other.sequence;
        }

        Segment& operator=(const Segment& other)
        {
            this->name = other.name;
            this->sequence = other.sequence;
            this->length = other.length;
            this->reads_count = other.reads_count;
            this->fragment_count = other.fragment_count;
            this->kmer_count = other.kmer_count;
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &output, const Segment &s ) { 
            output << "Unsupported";
            return output;     
        }

        friend std::istream &operator>>(std::istream  &input, Segment &s ) { 
            return input;
        }

        inline uint32_t GetLength() const {return length;}
        inline uint32_t GetReadCount() const {return reads_count;}
        inline uint32_t GetFragmentCount() const {return fragment_count;}
        inline uint32_t GetKmerCount() const {return kmer_count;}
        inline const std::string& GetName() const {return name;}
        inline const SequenceString& GetSequence() const {return sequence;}

        inline std::string str() const 
        {
            std::stringstream ss;
            ss << name << " " << sequence.str();
            return ss.str();
        }

};

}

#endif /*SEGMENT_HPP*/