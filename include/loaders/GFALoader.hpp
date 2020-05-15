#ifndef GFALOADER_H
#define GFALOADER_H

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <VG.hpp>
#include <Segment.hpp>
#include <ARGLoader.hpp>
namespace vflib
{

//TODO: Aggiungere in controlli su nodi e edge quando si inserisce (es. controllare se un nodo è nella mappa)
class StreamGFALoader: public StreamARGLoader<Segment, VGEdge>
{
    private:
        const size_t MIN_SEGMENT_SIZE = 3; //Minumum number of values for segment line
        const size_t MIN_LINK_SIZE = 6; //Minumum number of values for link line
        const std::string POSITIVE_ORIENTATION = "+";
        const std::string NEGATIVE_ORIENTATION = "-";

        std::unordered_map<std::string /*segment name*/, NodeId> name_to_id;
        std::unordered_map<std::string, VGPath> paths;

        std::vector<std::string> split(const std::string& s, char delimiter)
        {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(s);
            while (std::getline(tokenStream, token, delimiter))
            {
                tokens.push_back(token);
            }
            return tokens;
        }

        inline void printUnsupported(const char& linetype)
        {
            std::cout << "Unsupported line type " << linetype;
        }

        void readHeader(const std::string& line)
        {
            return;
        }

        void readNode(const std::string& line)
        {
            std::cout << "Reading node" << std::endl;
            std::vector<std::string> tokens = split(line, '\t');
            if(tokens.size() >= MIN_SEGMENT_SIZE){
                std::string name = tokens[1];
                std::string sequence_base = tokens[2];
                SequenceString sequence(sequence_base);
                Segment segment(name, sequence);
                name_to_id[name]=this->InsertNode(segment);
                std::cout << name << " " << sequence_base << " " << name_to_id[name] << std::endl; 
            }
        }

        void readEdge(const std::string& line)
        {
            std::cout << "Reading edge" << std::endl;
            std::vector<std::string> tokens = split(line, '\t');
            if(tokens.size() >= MIN_LINK_SIZE){
                std::string node1 = tokens[1];
                std::string node2 = tokens[3];
                std::string orientation1 = tokens[2];
                std::string orientation2 = tokens[4];
                std::string cigar = tokens[5];
                std::cout << node1 << " " << orientation1 << " " << node2 << " " << orientation2 << " " << cigar << std::endl; 
                NodeId id1 = name_to_id[node1];
                NodeId id2 = name_to_id[node2];
                EdgeOrientation etype = EdgeOrientation::FORWARD;
                //Computing edge orientation
                if(orientation1==POSITIVE_ORIENTATION && 
                    orientation1==NEGATIVE_ORIENTATION)
                {
                    etype=EdgeOrientation::FORWARD_REVERSE;
                }
                
                else if(orientation1==NEGATIVE_ORIENTATION && 
                    orientation1==POSITIVE_ORIENTATION)
                {
                    etype=EdgeOrientation::REVERSE;
                }
                else
                {
                    //Unknown orientation
                }

                VGEdge edge(cigar,etype);
                this->InsertEdge(id1, id2, edge);
            }
        }

        void readPath(const std::string& line)
        {
            std::cout << "Reading path ";
            std::vector<std::string> tokens = split(line, '\t');
            std::string name = tokens[1];
            std::cout << "Name: " << name << std::endl;
            VGPath path(name);
            std::cout << "Nodes: ";
            std::vector<std::string> nodetokens = split(tokens[2], '\t');
            for (auto ntks : nodetokens)
            {
                std::vector<std::string> nodes = split(ntks, ',');
                for (auto ntk : nodes)
                {
                    std::string node = ntk.substr(0, ntk.size()-1);
                    std::string orientation = ntk.substr(node.size()-1, ntk.size());
                    //std::string orientation = POSITIVE_ORIENTATION;
                    SegmentOrientation o;
                    if(orientation == POSITIVE_ORIENTATION)
                    {
                        o = SegmentOrientation::FORWARD;
                    }
                    else if (orientation == NEGATIVE_ORIENTATION)
                    {
                        o = SegmentOrientation::REVERSE;
                    }
                    else
                    {                
                        o = SegmentOrientation::UNDEFINED;
                    }
                    std::cout << node << " " << orientation << " ";
                    path.AddSegment(name_to_id[node], o);
                }
            }
            std::cout << std::endl << "Overlaps: ";
            std::string cigar = tokens[3];
            std::vector<std::string> ovtokens = split(cigar, ',');
            
            for (auto ov : ovtokens)
            {
                std::cout << ov << " ";
                path.AddOverlap(ov);
            }
            std::cout << std::endl;
            paths[name] = std::move(path);
        }

    public:
        StreamGFALoader(std::istream &in):
            StreamARGLoader<Segment, VGEdge>(in)
            {
                std::string line;
                char linetype = 'U'; //U as Undefined (not in types of GFA1)
                while(!in.eof())
                {
                    if(in.good())
                    {
                        /*
                        * Line types
                        * Type 	Description
                        *   # 	Comment
                        *   H 	Header
                        *   S 	Segment
                        *   L 	Link
                        *   C 	Containment
                        *   P 	Path
                        */
                        std::getline(in, line);
                        if(!(line.empty() || line[0]=='#'))
                        {
                            std::cout << "Reading line " << line << std::endl;
                            linetype = line[0]; 
                            switch(linetype)
                            {
                                case 'H':
                                    readHeader(line);
                                    break;
                                case 'S':
                                    readNode(line);
                                    break;
                                case 'L':
                                    readEdge(line);
                                    break;
                                case 'C':
                                    printUnsupported(linetype);
                                    break;
                                case 'P':
                                    readPath(line);
                                    break;
                                default:
                                    printUnsupported(linetype);
                                    break;
                            }
                        }
                    }
                }
            }

        
        void GetPaths(std::unordered_map<std::string, VGPath>& other)
        {
            using it = std::unordered_map<std::string, VGPath>::iterator;
            using mv = std::move_iterator <it>;
            other.insert(mv(paths.begin()), mv(paths.end()));
        }

};

}
#endif /*GFALOADER_H*/