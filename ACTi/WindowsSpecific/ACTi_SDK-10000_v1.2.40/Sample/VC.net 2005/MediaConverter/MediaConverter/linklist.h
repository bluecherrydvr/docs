#pragma once
//#ifndef LINKLIST_H
//	#define LINKLIST_H
#include <windows.h>
template <class TYPE> class linklist 
{
private:
	typedef struct node 
	{
		//this linklist uses POINTERS to data, so when adding a node you should always specify a new space in memory. 
		//The linklist will delete the space automatically when the node is removed or the linklist is destructed.
		TYPE *data;
		node *next;
	};
	node *first, *mark, *last;					//mark = address of the node accessed from the list
	unsigned int last_index;					//index of mark
	unsigned int size;							//size of list

public:

	//Constructors
	linklist();
	linklist(const TYPE &);						//initialize a list by allocating a new spot in memory and calling the TYPE's copy constructor
	linklist(TYPE *);							//initialize a list with an object already defined in memory
	linklist(linklist<TYPE> &);					//copy constructor

	//Destructors
	~linklist();

	//Operations - Navigation
	linklist<TYPE>& Next();						//next node in list
	linklist<TYPE>& First();					//first or head node of list
	linklist<TYPE>& Last();						//last node in the list
	linklist<TYPE>& Current();					//Not necessary, but implemented so code could be read more easily
	linklist<TYPE>& GoTo(unsigned int);			//Use this to go to a specific node number

	//Operations - Editing
	bool Add(const TYPE &, bool=false);			//Adds a new node by allocating a new spot in memory and calling the TYPE's copy constructor, bool value should be true to insert at head
	bool Add(TYPE *, bool=false);				//Adds a new node with the object already defined in memory, bool value should be true to insert at head
	bool Remove();								//returns true/false if node was removed
	void Empty();								//removes all nodes from the list
	void Swap(unsigned int);					//swap the data in two nodes

	//Operations - Info (all const)
	int Count() const;
	TYPE* Addr() const;
	bool Exists() const;

	//Overloaded operators
	linklist<TYPE>& operator[](unsigned int);	//Refencing a specific node by index
	void operator=(const TYPE &);				//Easily set a node's data using '='
	void operator=(TYPE *);						//Easily set a node's data using '='
	void operator=(linklist<TYPE> &);			//Easily copy an entire linklist using '='
	operator TYPE() const;						//conversion of node to TYPE, typically for cout usage
};

template <class TYPE> linklist<TYPE>::linklist() : first(NULL), mark(NULL), size(0), last_index(0) {}
template <class TYPE> linklist<TYPE>::linklist(const TYPE &d) : first(NULL), mark(NULL), size(0), last_index(0) { Add(d); }
template <class TYPE> linklist<TYPE>::linklist(TYPE *d) : first(NULL), mark(NULL), size(0), last_index(0) { Add(d); }
template <class TYPE> linklist<TYPE>::linklist(linklist<TYPE> &copylist) : first(NULL), mark(NULL), size(0), last_index(0) { *this = copylist; }

template <class TYPE> bool linklist<TYPE>::Add(const TYPE &d, bool front) 
{
	TYPE *new_d = new TYPE(d);
	if(new_d == NULL) return false;
	return Add(new_d, front);
}

template <class TYPE> bool linklist<TYPE>::Add(TYPE *d, bool front) 
{
	if(d == NULL)
		return false;

	node *new_node = new node;

	if(new_node == NULL) 
		return false;

	new_node->data = d;

	//Add a node making it the first node
	if(front) 
	{ 
		new_node->next = first;
		// Jo_20060525 - add last pointer(S)
		if(NULL == first)
		{
			last = new_node;
		}
		// Jo_20060525 - add last pointer(E)
		first = new_node;
		last_index = 0;
	}
	//if the list as no elements
	else if(first == NULL) 
	{ 
		first = new_node;
		// Jo_20060525 - add last pointer(S)
		last = new_node;
		// Jo_20060525 - add last pointer(E)
		first->next = NULL;
		last_index = 0;
	} 

	else 
	{ //otherwise, add it at the last mark index, or at the end if mark is null
		/*if(mark == NULL) 
			Last();*/
		// Jo_20060525 - Replace above code(S)
		mark = last;
		// Jo_20060525 - Replace above code(E)

		new_node->next = mark->next;
		mark->next = new_node;

		// Jo_20060525 - last points to correct node(S)
		last = new_node;
		// Jo_20060525 - last points to correct node(E)

		last_index++;
	}

	mark = new_node;
	size++;
	return true;
}

// Jo_20060525 - Do not use this function in ARAW
template <class TYPE> void linklist<TYPE>::Swap(unsigned int swap)
{
	if( mark == NULL || swap == last_index ) 
		return;
	
	node *find;
	unsigned int find_index;

	if( last_index > swap ) 
	{
		find = first;
		find_index = 0;
	}
	
	else 
	{
		find = mark;
		find_index = last_index;
	}

	//Do not use GoTo(), it will modifiy the mark pointer!
	for( ; find != NULL && find_index < swap; find = find->next ) 
		find_index++;

	if(find == NULL) 
		return;

	TYPE *tmp = find->data;
	find->data = mark->data;
	mark->data = tmp;
}

// Jo_20060525 - Do not use this function in ARAW
template <class TYPE> bool linklist<TYPE>::Remove() 
{
	if(mark != NULL) 
	{
		//special case: remove first node (set to next in line)
		if(first == mark) 
		{ 
			first = (first->next == NULL) ? NULL : first->next;
			delete mark->data;
			delete mark;
			mark = first;
			size--;
		} 
		
		else 
		{
			//otherwise, keep the address of the current node, just change delete its data and
			//point it to the next node's data, then delete the next node.
			//this eliminates the need to find the node before this, which would require a search
			//from the first node onward
			delete mark->data;
			node *temp;

			//only if it's the last node do we have to search the list for the node before it
			if(mark->next == NULL) 
			{ 
				for(temp = first; temp->next != NULL; temp = temp->next)
				{
					if(temp->next == mark) 
					{
						temp->next = (mark->next == NULL) ? NULL : mark->next;
						delete mark;
						mark = temp->next;
						size--;
						break;
					}
				}
			} 

			else 
			{
				mark->data = mark->next->data;
				temp = mark->next->next;
				delete mark->next;
				mark->next = temp;
			}
		}
		return true; //node was successfully removed
	}
	return false; //node wasn't found
}

template <class TYPE> int linklist<TYPE>::Count() const 
{ 
	return size; 
}

template <class TYPE> bool linklist<TYPE>::Exists() const 
{ 
	return (mark != NULL); 
}

template <class TYPE> linklist<TYPE>::~linklist() 
{
	while(first != NULL) 
	{
		mark = first;
		first = first->next;
		delete mark->data;
		delete mark;
	}
	// Jo_20060525 - set last to NULL(S)
	last = NULL;
	// Jo_20060525 - set last to NULL(E)
}

template <class TYPE> void linklist<TYPE>::Empty() 
{
	this->~linklist();
	first = NULL;
	mark = NULL;
	// Jo_20060525 - set last to NULL(S)
	last = NULL;
	// Jo_20060525 - set last to NULL(E)
	size = 0;
}

template <class TYPE> linklist<TYPE>::operator TYPE() const 
{
	if(mark != NULL)
		return *mark->data;
	else
		return NULL;
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::Next() 
{
	if(mark != NULL) 
	{
		mark = mark->next;
		last_index++;
	}
	return *this;
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::First() 
{
	if(first != NULL) 
	{
		mark = first;
		last_index = 0;
	}
	return *this;
}

template <class TYPE> TYPE* linklist<TYPE>::Addr() const 
{ 
	if(mark != NULL) 
		return mark->data;

	else
		return NULL; 
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::Current() 
{ 
	return *this; 
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::Last() 
{ 
	//return (*this)[size - 1]; 
	if(last != NULL) 
	{
		mark = last;
		last_index = size;
	}
	return *this;
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::GoTo(unsigned int index) 
{ 
	return (*this)[index]; 
}

template <class TYPE> linklist<TYPE>& linklist<TYPE>::operator[](unsigned int index) 
{
	if(last_index > index) 
		First();

	for(; mark != NULL && last_index < index; mark = mark->next) 
		last_index++;

	return *this;
}

template <class TYPE> void linklist<TYPE>::operator=(const TYPE &d) 
{
	if(mark == NULL) 
		return;

	TYPE *new_data = new TYPE(d);

	if(new_data == NULL) 
		return;

	delete mark->data;
	mark->data = new_data;
}

template <class TYPE> void linklist<TYPE>::operator=(TYPE *d) 
{
	if(mark == NULL) 
		return;

	if(d == NULL) 
		return;

	delete mark->data;
	mark->data = d;
}

template <class TYPE> void linklist<TYPE>::operator=(linklist<TYPE>& copylist) 
{
	Empty();
	for(node *temp = copylist.first; temp != NULL; temp = temp->next)
	{
		if(!Add(*temp->data)) 
			break;
	}
}

//#endif LINKLIST_H
