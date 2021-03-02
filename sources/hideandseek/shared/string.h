class String
{
private:
	char	*_array;
	uint32	_count; // NOTE: count includes \0

protected:
	void Initialize (const char *str, int len = -1);
	void Destroy ();

public:
	String();
	String (nullptr_t);
	String(const char *str);
	String (const String &str);
	~String();

	void Clear ();
	String Clone () const;
	String &Concatenate (const char *str, int length = -1);
	String &Concatenate (const String &str);
	String &Remove (const int startIndex, const int length = -1);
	String Substring (const int startIndex, const int length = -1) const;
	int Compare (const char *str, int maxLength = -1) const;
	int CompareCaseInsensitive (const char *str, int maxLength = -1) const;
	int Compare (const String &str, int maxLength = -1) const;
	int CompareCaseInsensitive (const String &str, int maxLength = -1) const;
	// FIXME: insensitive versions
	int IndexOf (char ch, const int offset = 0) const;
	int IndexOf(const String &str, const int offset = 0) const;
	int IndexOf(const char *str, const int offset = 0) const;
	int LastIndexOf (const char ch, const int offset = 0) const;
	int LastIndexOf(const String &str, const int offset = 0) const;
	int LastIndexOf(const char *str, const int offset = 0) const;
	// FIXME: insensitive versions
	bool Contains (const String &str) const;
	bool Contains (const char *str) const;
	// FIXME: insensitive versions
	bool EndsWith (const String &str) const;
	bool EndsWith (const char *str) const;
	// FIXME: insensitive versions
	bool StartsWith (const String &str) const;
	bool StartsWith(const char *str) const;
	String &Insert (const char *str, const int position);
	String &Insert (const String &str, const int position);
	String &Insert (char ch, const int position);
	static String Format (const char *format, ...);
	bool IsNullOrEmpty () const;
	static bool IsNullOrEmpty(const String &str);
	static bool IsNullOrEmpty (const String *str);
	bool operator== (const String &right) const;
	bool operator== (const char *right) const;
	bool operator!= (const String &right) const;
	bool operator!= (const char *right) const;
	String &operator= (const String &r);
	String &operator= (const char *r);
	String &operator+= (const String &r);
	String &operator+= (const char *r);
	String operator+ (const String &r);
	String operator+ (const char *r);
	String &operator+= (const char &ch);
	String operator+ (const char &ch);
	char &operator[] (const int &index) const;
	static String Join (int count, ...);
	static String Join (const String *list, int count = -1);
	static String Join (const TList<String> &list);
	String &Replace (const char from, const char to);
	String &Replace (const String &from, const String &to);
	String &Replace (const char *from, const char *to);
	bool IsNullOrWhiteSpace () const;

	template <typename T>
	static bool IsAny (const T &v, const char *list, int num)
	{
		for (int i = 0; i < num; ++i)
			if (v == list[i])
				return true;

		return false;
	}

	TList<String> Split (const char *characters, int count, bool removeEmpty) const;
	TList<String> Split (const TList<char> &characters, bool removeEmpty) const;
	TList<String> Split (bool removeEmpty = false) const;
	TList<char> ToCharArray () const;
	String &ToLower ();
	String &ToUpper ();
	String &Trim (int forceStart = -1, int forceEnd = -1);
	String &TrimStart ();
	String &TrimEnd ();
	uint32 Count() const;
	char At (int index) const;
	const char *CString() const;

	static const String &Empty();
};