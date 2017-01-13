#ifndef __SLIM_XML_H__
#define __SLIM_XML_H__

#include <string>
#include <list>
#include <istream>

//disable this to make parsing a little bit faster
//#define SLIM_TRANSFER_CHARACTER

namespace slim
{

#if defined (_MSC_VER) && defined (UNICODE)
	#define SLIM_USE_WCHAR
#endif

enum Encode
{
	ANSI = 0,
	UTF_8,
	UTF_8_NO_MARK,
	UTF_16,
	UTF_16_BIG_ENDIAN,

#if defined (SLIM_USE_WCHAR) || defined (__GNUC__)
	DefaultEncode = UTF_8
#else
	DefaultEncode = ANSI
#endif
};

#ifdef SLIM_USE_WCHAR
	typedef wchar_t Char;
	#define SXT(str) L##str
	#define StrToI _wtoi
	#define StrToF _wtof
	#define Sprintf swprintf
	#define Sscanf swscanf
	#define Strlen wcslen
	#define Strcmp wcscmp
	#define Strncmp wcsncmp
	#define Memchr wmemchr
	#define Strcpy wcscpy
#else
	typedef char Char;
	#define SXT(str) str
	#define StrToI atoi
	#define StrToF atof
#if defined (__GNUC__)
	#define Sprintf snprintf
#elif defined (_MSC_VER)
	#define Sprintf sprintf_s
#endif
	#define Sscanf sscanf
	#define Strlen strlen
	#define Strcmp strcmp
	#define Strncmp strncmp
	#define Memchr memchr
	#define Strcpy strcpy
#endif

class XmlAttribute;
class XmlNode;

typedef std::basic_string<Char> String;
typedef std::list<XmlAttribute*> AttributeList;
typedef std::list<XmlNode*> NodeList;

typedef AttributeList::const_iterator AttributeIterator;
typedef NodeList::const_iterator NodeIterator;

enum NodeType
{
	DOCUMENT = 0,
	ELEMENT,
	COMMENT,
	DECLARATION
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class XmlBase
{
	friend class XmlDocument;

public:
	XmlBase();
	~XmlBase();

	const Char* getName() const;
	void setName(const Char* name);

	template<typename T>
	T getValue() const;

	template<typename T>
	void setValue(const T& value);

	void setValue(const Char* value);

	unsigned long getHex() const;
	void setHex(unsigned long value);

private:
	void assignString(Char* &str, Char* value, size_t length, bool transferCharacter);

protected:
	Char*	m_name;
	Char*	m_value;
	bool	m_nameAllocated;
	bool	m_valueAllocated;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class XmlAttribute : public XmlBase
{
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class XmlNode : public XmlBase
{
public:
	XmlNode(NodeType type, XmlNode* parent);
	~XmlNode();

public:
	NodeType getType() const;

	bool isEmpty() const;

	XmlNode* getParent() const;

	bool hasChild() const;

	XmlNode* getFirstChild(NodeIterator& iter) const;
	XmlNode* getNextChild(NodeIterator& iter) const;
	XmlNode* getChild(NodeIterator iter) const;
	size_t getChildCount() const;

	XmlNode* findChild(const Char* name) const;
	XmlNode* findFirstChild(const Char* name, NodeIterator& iter) const;
	XmlNode* findNextChild(const Char* name, NodeIterator& iter) const;
	size_t getChildCount(const Char* name) const;

	void removeChild(XmlNode* node);
	void clearChild();

	XmlNode* addChild(const Char* name = NULL, NodeType = ELEMENT);

	bool hasAttribute() const;

	XmlAttribute* findAttribute(const Char* name) const;

	template<typename T>
	T readAttribute(const Char* name, T defaultValue) const;

	void readAttributeAsIntArray(const Char* name, int* out, unsigned long length, int defaultValue = 0) const;
	unsigned long readAttributeAsHex(const Char* name, unsigned long defaultValue = 0) const;
	unsigned long readAttributeAsEnum(const Char* name, const Char* const* enumNames,
									  unsigned long enumCount, unsigned long defaultValue = 0) const;

	XmlAttribute* getFirstAttribute(AttributeIterator& iter) const;
	XmlAttribute* getNextAttribute(AttributeIterator& iter) const;

	void removeAttribute(XmlAttribute* attribute);
	void clearAttribute();

	template<typename T>
	XmlAttribute* addAttribute(const Char* name, const T& value);

	XmlAttribute* addAttribute(const Char* name, const Char* value);

	XmlAttribute* addAttribute();

protected:
	void writeNode(String& output, int depth) const;

	void writeChildNodes(String& output, int depth) const;

	void writeTransferredString(String& output, const Char* input) const;

private:
	NodeType		m_type;
	AttributeList	m_attributes;
	XmlNode*		m_parent;
	NodeList		m_children;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class XmlDocument : public XmlNode
{
public:
	XmlDocument();
	~XmlDocument();

	bool loadFromFile(const Char* filename);
	bool loadFromStream(std::istream& input);
	bool loadFromMemory(const char* buffer, size_t size);

	bool save(const Char* filename, Encode encode = DefaultEncode) const;

private:
	bool reallyLoadFromMemory(char* buffer, size_t size, bool copiedMemory);

	bool parse(Char* input, size_t size);

	bool findLabel(Char* &begin, size_t size, Char* &label, size_t &labelSize);

	bool parseLabel(XmlNode* node, Char* label, size_t labelSize);

private:
	char*	m_buffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlBase::XmlBase()
	: m_name(const_cast<Char*>(SXT("")))
	, m_value(const_cast<Char*>(SXT("")))
	, m_nameAllocated(false)
	, m_valueAllocated(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlBase::~XmlBase()
{
	if (m_nameAllocated && m_name != NULL)
	{
		delete m_name;
	}
	if (m_valueAllocated && m_value != NULL)
	{
		delete m_value;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* XmlBase::getName() const
{
	return m_name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setName(const Char* name)
{
	if (m_nameAllocated && m_name != NULL)
	{
		delete[] m_name;
	}
	m_name = new Char[Strlen(name) + 1];
	Strcpy(m_name, name);
	m_nameAllocated = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline const Char* XmlBase::getValue() const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline bool XmlBase::getValue() const
{
	return (Strcmp(m_value, SXT("true")) == 0 ||
			Strcmp(m_value, SXT("TRUE")) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline int XmlBase::getValue() const
{
	return StrToI(m_value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline float XmlBase::getValue() const
{
	return static_cast<float>(StrToF(m_value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline double XmlBase::getValue() const
{
	return StrToF(m_value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setValue(const Char* value)
{
	if (m_valueAllocated && m_value != NULL)
	{
		delete[] m_value;
	}
	m_value = new Char[Strlen(value) + 1];
	Strcpy(m_value, value);
	m_valueAllocated = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline void XmlBase::setValue<String>(const String& value)
{
	if (m_valueAllocated && m_value != NULL)
	{
		delete[] m_value;
	}
	m_value = new Char[value.length() + 1];
	Strcpy(m_value, value.c_str());
	m_valueAllocated = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline void XmlBase::setValue<bool>(const bool& value)
{
	setValue(value ? SXT("true") : SXT("false"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline void XmlBase::setValue<int>(const int& value)
{
	Char sz[128];
	Sprintf(sz, sizeof(sz), SXT("%d"), value);
	setValue(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline void XmlBase::setValue<float>(const float& value)
{
	Char sz[128];
	Sprintf(sz, sizeof(sz), SXT("%g"), value);
	setValue(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline void XmlBase::setValue<double>(const double& value)
{
	Char sz[128];
	Sprintf(sz, sizeof(sz), SXT("%g"), value);
	setValue(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long XmlBase::getHex() const
{
	unsigned long value = 0;
	Sscanf(m_value, SXT("%X"), &value);
	if (value == 0)
	{
		Sscanf(m_value, SXT("%x"), &value);
	}
	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setHex(unsigned long value)
{
	Char sz[128];
	Sprintf(sz, sizeof(sz), SXT("%X"), (unsigned long)value);
	setValue(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline NodeType XmlNode::getType() const
{
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::isEmpty() const
{
	return (!hasChild() && (m_value == NULL || m_value[0] == 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::hasChild() const
{
	return !m_children.empty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getParent() const
{
	return m_parent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getFirstChild(NodeIterator& iter) const
{
	iter = m_children.begin();
	if (iter != m_children.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getNextChild(NodeIterator& iter) const
{
	if (iter != m_children.end())
	{
		++iter;
		if (iter != m_children.end())
		{
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getChild(NodeIterator iter) const
{
	if (iter != m_children.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t XmlNode::getChildCount() const
{
	return m_children.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::hasAttribute() const
{
	return !m_attributes.empty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlAttribute* XmlNode::getFirstAttribute(AttributeIterator& iter) const
{
	iter = m_attributes.begin();
	if (iter != m_attributes.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlAttribute* XmlNode::getNextAttribute(AttributeIterator& iter) const
{
	if (iter != m_attributes.end())
	{
		++iter;
		if (iter != m_attributes.end())
		{
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline T XmlNode::readAttribute(const Char* name, T defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getValue<T>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlAttribute* XmlNode::addAttribute(const Char* name, const Char* value)
{
	XmlAttribute* attribute = new XmlAttribute;
	if (name != NULL)
	{
		attribute->setName(name);
	}
	if (value != NULL)
	{
		attribute->setValue(value);
	}
	m_attributes.push_back(attribute);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline XmlAttribute* XmlNode::addAttribute(const Char* name, const T& value)
{
	XmlAttribute* attribute = addAttribute();
	attribute->setName(name);
	attribute->setValue<T>(value);
	return attribute;
}

size_t utf8toutf16(const char* u8, size_t size, wchar_t* u16, size_t outBufferSize);
size_t utf16toutf8(const wchar_t* u16, size_t size, char* u8, size_t outBufferSize);
Encode detectEncode(const char* str, size_t size, bool& multiBytes);

}

#endif
