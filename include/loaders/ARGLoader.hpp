/**
 * @file   ARGLoader.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   December, 2014
 * @see ARGraph.hpp
 * @brief  Definition of a simple ARG loader which allows graph edit operations.
 * @details DESCRIPTION OF THE BINARY FILE FORMAT \n
 * The file is composed by a sequence of 16-bit words; the words are
 * encoded in little-endian format (e.g., LSB first).\n
 * The first word represents the number of nodes in the graph.
 * Then, for each node, there is a word encoding the number of
 * edges coming out of that node, followed by a sequence of words
 * encoding the endpoints of those edges.\n
 * An example, represented in hexadecimal, follows:\n
 *     03 00     Number of nodes (3)\n
 *     00 00     Number of edges out of node 0 (0)\n
 *     02 00     Number of edges out of node 1 (2)\n
 *     00 00     Target of the first edge of node 1 (edge 1 -> 0)\n
 *     02 00     Target of the second edge of node 1 (edge 1 -> 2)\n
 *     01 00     Number of edges out of node 2 (1)\n
 *     00 00     Target of the first (and only) edge of node 2 (edge 2 -> 0)\n
*/

#ifndef ARGLOADER_H
#define ARGLOADER_H

#include <iostream>
#include <strstream>
#include <map>
#include <ctype.h>

#include <ARGEdit.hpp>

namespace vflib
{

	/**
	 * @class StreamARGLoader
	 * @brief ARGLoader for text file format.
	 *
	 * DESCRIPTION OF THE TEXT FILE FORMAT
	 * ===================================
	 *
	 * On the first line there must be the number of nodes;
	 * subsequent lines will contain the node attributes, one node per
	 * line, preceded by the node id; node ids must be in the range from
	 * 0 to the number of nodes - 1.\n
	 * Then, for each node there is the number of edges coming out of
	 * the node, followed by a line for each edge containing the
	 * ids of the edge ends and the edge attribute.\n
	 * Blank lines, and lines starting with #, are ignored.
	 * An example file, where both node and edge attributes are ints,
	 * could be the following:
	 *
	 *	\# Number of nodes\n
	 *	3\n
	 *
	 *	\# Node attributes\n
	 *	0 27\n
	 * 	1 42\n
	 *	2 13\n
	 *
	 *	\# Edges coming out of node 0\n
	 *	2\n
	 *	0 1  24\n
	 *	0 2  73\n
	 *
	 *	\# Edges coming out of node 1\n
	 *	1\n
	 *	1 3  66\n
	 *
	 *	\# Edges coming out of node 2\n
	 *	0\n
	 *
	 */
	template <typename Node, typename Edge>
	class StreamARGLoader : public ARGEdit<Node, Edge>
	{
	public:
		StreamARGLoader(std::istream &in, bool force_undirected=false);
		static void write(std::ostream &out, ARGraph<Node, Edge> &g);
		static void write(std::ostream &out, ARGLoader<Node, Edge> &g);

	private:
		enum { MAX_LINE = 512 };
		bool force_undirected;
		std::map<nodeID_t, std::map<nodeID_t, Edge >> edges_map; //only to force undirected load
		void readLine(std::istream &in, char *line);
		int64_t  readCount(std::istream &in);
		void readNode(std::istream &in);
		void readEdge(std::istream &in);
	};

	/*------------------------------------------------------------
	 * Methods of the class StreamArgLoader
	 -----------------------------------------------------------*/

	 /**
	 * @brief Constructor
	 * @param [in] in Input stream
	 */
	template <typename Node, typename Edge>
	StreamARGLoader<Node, Edge>::
		StreamARGLoader(std::istream &in, bool force_undirected):
		force_undirected(force_undirected)
	{
		int64_t cnt = readCount(in);
		if (cnt <= 0)
		{
			cnt = 0;
			return;
		}

		int64_t i;
		for (i = 0; i < cnt; i++)
		{
			readNode(in);
		}

		for (i = 0; i < cnt; i++)
		{
			int64_t ecount, j;
			ecount = readCount(in);
			for (j = 0; j < ecount; j++)
				readEdge(in);
		}
		if (force_undirected)
		{
			for(nodeID_t n1 = 0; n1<cnt; n1++)
			{
				for(typename std::map<nodeID_t, Edge >::iterator iter = edges_map[n1].begin();
					iter != edges_map[n1].end(); ++iter)
				{
					this->InsertEdge(n1, iter->first, iter->second);
				}
			}
		}
		

	}

	/**
	* @brief Reads a line of the stream
	* @param [in] in Input stream.
	* @param [out] line Read line. You should not allocate this.
	*/
	template <typename Node, typename Edge>
	void StreamARGLoader<Node, Edge>::
		readLine(std::istream &in, char *line)
	{
		char *p;
		do {
			*line = '\0';
			if (!in.good())
				error("End of file or reading error");
			in.getline(line, MAX_LINE);
			for (p = line; isspace(*p); p++)
				;
		} while (*p == '\0' || *p == '#');
	}

	/**
	* @brief Reads an int64_t from a line
	* @param [in] in Input stream.
	* @returns Integer read.
	*/
	template <typename Node, typename Edge>
	int64_t StreamARGLoader<Node, Edge>::
		readCount(std::istream &in)
	{
		char line[MAX_LINE + 1];
		readLine(in, line);

		int64_t i;
		std::istrstream is(line);
		is >> i;

		return i;
	}

	/**
	* @brief Loads a node from a line.
	* @param [in] in Input stream.
	*/
	template <typename Node, typename Edge>
	void StreamARGLoader<Node, Edge>::
		readNode(std::istream &in)
	{
		char line[MAX_LINE + 1];
		readLine(in, line);
		std::istrstream is(line);

		Node nattr;
		nodeID_t id;

		is >> id >> nattr;

		if (id != this->NodeCount())
			error("File format error\n  Line: %s", line);

		this->InsertNode(nattr);
	}

	/**
	* @brief Loads an edge from a line
	* @param [in] in Input stream.
	*/
	template <typename Node, typename Edge>
	void StreamARGLoader<Node, Edge>::
		readEdge(std::istream &in)
	{
		char line[MAX_LINE + 1];
		readLine(in, line);
		std::istrstream is(line);

		Edge eattr;
		nodeID_t id1, id2;

		is >> id1 >> id2 >> eattr;
						
		if(force_undirected)
		{
			edges_map[id1][id2]=eattr;
			
			if(edges_map[id2].find(id1) == edges_map[id2].end())
			{
				Edge eattr2 = eattr;
				edges_map[id2][id1] = eattr2;
			}
		}
		else
		{
			this->InsertEdge(id1, id2, eattr);	
		}
	}

	/**
	* @brief Writes an ARGraph on a stream in a format readable by StreamARGLoader.\n
	* Relies on stream output operators for the Node and Edge types.
	* @param [in] out Output stream.
	* @param [in] g ARGraph to write.
	*/
	template <typename Node, typename Edge>
	void StreamARGLoader<Node, Edge>::
		write(std::ostream &out, ARGraph<Node, Edge> &g)
	{
		out << g.NodeCount() << std::endl;

		uint32_t i;
		for (i = 0; i < g.NodeCount(); i++)
			out << i << ' ' << *g.GetNodeAttr(i) << std::endl;

		uint32_t j;
		for (i = 0; i < g.NodeCount(); i++)
		{
			out << g.OutEdgeCount(i) << std::endl;
			for (j = 0; j < g.OutEdgeCount(i); j++)
			{
				uint32_t k;
				Edge *attr;
				k = g.GetOutEdge(i, j, &attr);
				out << i << ' ' << k << ' ' << *attr << std::endl;
			}
		}
	}

	/**
	* @brief Writes an ARGLoader on a stream in a format readable by StreamARGLoader.\n
	* Relies on stream output operators for the Node and Edge types.
	* @param [in] out Output stream.
	* @param [in] g ARGLoader to write.
	*/
	template <typename Node, typename Edge>
	void StreamARGLoader<Node, Edge>::
		write(std::ostream &out, ARGLoader<Node, Edge> &g)
	{
		out << g.NodeCount() << std::endl;

		uint32_t i;
		for (i = 0; i < g.NodeCount(); i++)
			out << i << ' ' << *(Node *)g.GetNodeAttr(i) << std::endl;

		uint32_t j;
		for (i = 0; i < g.NodeCount(); i++)
		{
			out << g.OutEdgeCount(i) << std::endl;
			for (j = 0; j < g.OutEdgeCount(i); j++)
			{
				uint32_t k;
				void *attr;
				k = g.GetOutEdge(i, j, &attr);
				out << i << ' ' << k << ' ' << *(Edge *)attr << std::endl;
			}
		}
	}
}

#endif
