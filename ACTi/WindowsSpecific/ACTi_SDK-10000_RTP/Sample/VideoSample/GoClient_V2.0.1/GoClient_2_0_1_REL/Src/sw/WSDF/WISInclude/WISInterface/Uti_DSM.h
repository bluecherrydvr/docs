/**	HEADERFILE: "WSDF/WISInclude/WISInterface/Uti_DSM.h"
 *	Reference Doc: "TreeLink Data Structure Specification", Alpha
 *	Description: Data Structure Management Tools.
 *	Section: Utilities
 *	History:
 *		04-19-2002 - Alpha, file created
 */
#ifndef	_UTI_DSM_H_
#define	_UTI_DSM_H_

#include	<../WSDF.H>
#include	<../Utilities.H>



/**	SECTION - constants and data exchange structures
 */
	typedef enum
	{
		ERRDSM_EMPTYTREE	= 0x0401
	} EDSM_FuncError;

	typedef struct
	{
		SINT32	nchildren;		// count of child-nodes
		SINT32	current_child;	// index of current child-node
		SINT8	dft;			// if it's a default node (modification not allowed)
	} TDSM_NodePrefix;

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration for C++
 */
#ifdef __cplusplus
	class IDSM_TreeLink
	{
		SINT32 id;			// handle pointing to a CTreeLink object

		public:
		// constructor
				IDSM_TreeLink(
					UINT8	*treeData,		// memory buffer of the whole tree
					SINT32	*nodeSize,		// array of node structure size of each level
					SINT32	depth			// depth of the tree
							= 1				//   by default set tree depth to 1 (link list)
					);

		// distructor
				~IDSM_TreeLink();

		// to update current tree
		SINT32	UpdateTreeSize();
		SINT32	UpdateTreeData(
					UINT8	*treeData		// outside allocated buffer to receive tree data
					);
		// return actual size of tree

		// to get current path
		SINT32	GetCurPath(
					SINT32	*nodePath		// outside allocated buffer to receive node path
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to get prefix info of a node, will not change current
		SINT32	NodePrefix(
					TDSM_NodePrefix	*pfx,	// outside allocated buffer to receive node prefix
					SINT32	lvl				// 0-based node level (- 1 for root)
							= IDX_ROOT,		//   by default we return root prefix
					SINT32	*nodePath		// array of node index of each level
							= NULL,			//   by default select current node ('current_child')
					SINT32	idx				// node index at chosen level
							= FP_KEEP		//   by default we ignore it (use nodePath instead)
											//   FP_AUTO will choose first node
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to select a node, change to current
		SINT32	NodeSelect(
					UINT8	*nodeData,		// outside allocated buffer to receive node data
					SINT32	lvl				// 0-based node level
							= IDX_TAIL,		//   by default we select leaves
					SINT32	*nodePath		// array of node index of each level
							= NULL,			//   by default select current node ('current_child')
					SINT32	idx				// node index at chosen level
							= FP_KEEP		//   by default we ignore it (use nodePath instead)
											//   FP_AUTO will choose first node
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to insert a node, change to current
		SINT32	NodeInsert(
					UINT8	*nodeData,		// new node data to be inserted
					SINT32	lvl				// 0-based node level
							= IDX_TAIL,		//   by default we insert leaves
					SINT32	*nodePath		// array of node index of each level
							= NULL,			//   by default insert after current node
					SINT32	idx				// node index at chosen level
							= FP_KEEP,		//   by default we ignore it (use nodePath instead)
											//   FP_AUTO will insert to last node
					SINT8	dft				// if it's a default node
							= 0				//   by default we insert common nodes
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to update a node, change to current
		SINT32	NodeUpdate(
					UINT8	*nodeData,		// new node data to be modified
					SINT32	lvl				// 0-based node level
							= IDX_TAIL,		//   by default we update leaves
					SINT32	*nodePath		// array of node index of each level
							= NULL			//   by default update current node ('current_child')
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to delete a node, change to current
		SINT32	NodeDelete(
					SINT32	lvl				// 0-based node level
							= IDX_TAIL,		//   by default we delete leaves
					SINT32	*nodePath		// array of node index of each level
							= NULL			//   by default delete current node ('current_child')
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to rotate a node, change to current
		SINT32	NodeRotate(
					SINT32	forward,		// how many steps will the node forward (rotate)
					SINT32	lvl				// 0-based node level
							= IDX_TAIL,		//   by default we rotate leaves
					SINT32	*nodePath		// array of node index of each level
							= NULL			//   by default rotate current node ('current_child')
					);
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError

		// to get data headers (name etc.) of all children of a node, will not change current
		UINT8*	HeaderList(
					TDSM_NodePrefix	*pfx,	// outside allocated buffer to receive node prefix
					SINT32	hdSize,			// size of node data header
					SINT32	lvl				// 0-based node level (- 1 for root)
							= IDX_ROOT,		//   by default we return root prefix
					SINT32	*nodePath		// array of node index of each level
							= NULL,			//   by default select current node ('current_child')
					SINT32	idx				// node index at chosen level
							= FP_KEEP		//   by default we ignore it (use nodePath instead)
											//   FP_AUTO will choose first node
					);
		// return a non-zero pointer of newly allocated header list (need delete outside)

		// to get real data of all children of a node, will not change current
		SINT32  ListSubNode(
					UINT8 *subnode,			// outside allocated buffer to receive node data
					SINT32 lvl				// 0-based node level (- 1 for root)
							= IDX_ROOT,		//   by default we return root prefix
					SINT32 *nodePath		// array of node index of each level
							= NULL,			//   by default select current node ('current_child')
					SINT32 idx				// node index at chosen level
							= FP_KEEP		//   by default we ignore it (use nodePath instead)
											//   FP_AUTO will choose first node
					);
		// return number of children of a node
		
	};
#endif

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "Uti_DSM.h"
 */
