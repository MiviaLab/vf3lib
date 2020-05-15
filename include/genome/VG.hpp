#ifndef VG_HPP
#define VG_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <ARGraph.hpp>


namespace vflib
{

	enum class SegmentOrientation
	{
		FORWARD = 1,
		REVERSE = 2,
		UNDEFINED = 3
	};

	enum class EdgeOrientation
	{
		FORWARD = SegmentOrientation::FORWARD, // Forward to forward (Linear Sequence) (+/+)
		REVERSE = SegmentOrientation::REVERSE, // Reverse Complement to Reverse Complement (-/+)
		FORWARD_REVERSE = 3 // Forward to Reverse Complement (+/-)
	};

	class VGEdge
	{
	private:
		EdgeOrientation etype;
		std::string overlaps;

	public:
		VGEdge(std::string overlaps,
			EdgeOrientation etype = EdgeOrientation::FORWARD)
			:etype(etype), overlaps(overlaps) {}

		VGEdge(EdgeOrientation etype = EdgeOrientation::FORWARD)
			:etype(etype) {}

		
        friend std::ostream &operator<<(std::ostream &output, const VGEdge &s ) { 
            output << "Unsupported";
            return output;     
        }

        friend std::istream &operator>>(std::istream  &input, VGEdge &s ) { 
            return input;
        }

		inline std::string str() const {
            std::stringstream ss;
			std::string orient = "+";
			if(etype == EdgeOrientation::REVERSE)
			{
				orient = "-";
			}

			if(etype == EdgeOrientation::FORWARD_REVERSE)
			{
				orient = "+R";
			} 
            ss << orient;
            return ss.str();
        }

	};

	class VGPath
	{
	public:
		typedef std::pair<NodeId, SegmentOrientation> PathItem;

	private:
		std::string name;
		std::vector<PathItem> items;
		std::vector<std::string> overlaps;

	public:
		VGPath(std::string name) :name(name) {}

		VGPath():VGPath(""){}

		VGPath(std::string name,
			const std::vector<PathItem>& items,
			const std::vector<std::string>& overlaps) :
			name(name),
			items(items),
			overlaps(overlaps) {}

		VGPath(const VGPath& other) :
			name(other.name),
			items(other.items),
			overlaps(other.overlaps) {}

		VGPath(const VGPath&& other) :
			name(std::move(other.name)),
			items(std::move(other.items)),
			overlaps(std::move(other.overlaps)) {}

		bool operator==(const VGPath& other) const
        {
            return name == other.name;
        }

		bool operator!=(const VGPath& other) const
        {
            return *this != other;
        }

        VGPath& operator=(const VGPath& other)
        {
            (*this).name = other.name;
            (*this).items = other.items;
            (*this).overlaps = other.overlaps;
            return (*this);
        }

        VGPath& operator=(const VGPath&& other)
        {
            (*this).name = std::move(other.name);
            (*this).items = std::move(other.items);
            (*this).overlaps = std::move(other.overlaps);
            return (*this);
        }

		inline size_t GetLength() const { return items.size(); }
		inline void AddSegment(NodeId s, SegmentOrientation o)
		{
			items.push_back(std::make_pair(s, o));
		}
		inline void AddOverlap(const std::string& o) { overlaps.push_back(o); }
		inline const std::string& GetName() const { return name; }
		inline const std::vector<PathItem>& GetItems() const { return items; }
		inline const std::vector<std::string>& GetOverlaps() const { return overlaps; }

		inline std::string str() const {
            std::stringstream ss;

			for(auto item : items)
			{
				std::string orient = "+";
				if(item.second == SegmentOrientation::REVERSE)
				{
					orient = "-";
				} 	
				ss << item.first << orient << ", ";
			}
            return ss.str();
        }

	};

}

#endif //VG_HPP

