/*
Copyright (C) 2018-2020 Theodorus Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef _TAH_AvlTree_h_
#define _TAH_AvlTree_h_

#include <Common/Config.h>
#include <Common/container/Array.h>
#include <Common/Math.h>

#define NODE_NIL 0xFFFF
#define _avl_printf

namespace tas
{

template<class T1, class T2>
struct AvlTreeNode
{
	T1 key;
	T2 value;

	half parent;
	half left; /// indexes in tree array
	half right; /// index next free node
	char bactor; /// balance factor

	AvlTreeNode()
	{
		parent = NODE_NIL;
		left = NODE_NIL;
		right = NODE_NIL;
		bactor = 0;
	}
	AvlTreeNode(T1 _key, T2 _val)
	{
		key = _key;
		value = _val;
		parent = NODE_NIL;
		left = NODE_NIL;
		right = NODE_NIL;
		bactor = 0;
	}
	~AvlTreeNode()
	{
	}
	AvlTreeNode& operator = (const AvlTreeNode& node)
	{
		parent = node.parent;
		left = node.left;
		right = node.right;
		key = node.key;
		value = node.value;
		bactor = node.bactor;
		return *this;
	}
	AvlTreeNode& move(AvlTreeNode& node)
	{
		parent = node.parent;
		left = node.left;
		right = node.right;
		key = node.key;
		bactor = node.bactor;
#ifdef AVL_TREE_NOBJ
		value = node.value;
		node.value = 0;
#else
		value.move(node.value);
#endif
		node.parent = NODE_NIL;
		node.left = NODE_NIL;
		node.right = NODE_NIL;
		node.bactor = 0;
		return *this;
	}
	void reset()
	{
		parent = NODE_NIL;
		left = NODE_NIL;
		right = NODE_NIL;
		bactor = 0;
	}
};

template<class T1, class T2>
class AvlTree
{
public:
	typedef AvlTreeNode<T1, T2> node_type;
	ArrayMove<node_type, AllocatorObj<node_type> > tree;
	half root;
	half parent;
	half firstAvail;
	half totalAvail;

	AvlTree()
	{
		root = NODE_NIL;
		parent = NODE_NIL;
		firstAvail = 0;
		totalAvail = 0;
	}

	~AvlTree()
	{
	}

	void reset()
	{
		root = NODE_NIL;
		parent = NODE_NIL;
		firstAvail = 0;
		totalAvail = tree.capacity();
		if(totalAvail)
		{
			forn(totalAvail)
			tree[i].right = i + 1;
		}
	}

	uint size()
	{
		return tree.capacity() - totalAvail;
	}

	uint capacity()
	{
		return tree.capacity();
	}

	void resize(uint n)
	{
		if(n < tree.capacity())
			return;
		half inSize = size();
		half neSize = n;
		half neCount = neSize - inSize;
		tree.resize(neSize);
		forn(neCount)
		{
			tree[inSize + i].right = inSize + i + 1;
		}
		totalAvail = neCount;
	}

	half getRoot()
	{
		return root;
	}

	node_type& getRootNode()
	{
		return tree[root];
	}

	half getFreeNode()
	{
		if(totalAvail == 0)
		{
			half inSize = size();
			half neSize = inSize == 0 ? 2 : inSize * 2;
			resize(neSize);
		}
		half node = firstAvail;
		firstAvail = tree[node].right;
		tree[node].reset();
		totalAvail--;
		return node;
	}

	/// Insert node.
	half insert(T1 key, T2 val, half node = NODE_NIL)
	{
		if(node == NODE_NIL)
			node = root;
		parent = NODE_NIL;
		while(node != NODE_NIL)
		{
			parent = node;
			if(key < tree[node].key)
				node = tree[node].left;
			else
				node = tree[node].right;
		}

		node = getFreeNode();
		tree[node].key = key;
		tree[node].value = val;
		tree[node].parent = parent;
		tree[node].bactor = 0;

		if(parent == NODE_NIL)
		{
			root = node;
			return node;
		}
		else if(key < tree[parent].key)
			tree[parent].left = node;
		else
			tree[parent].right = node;

		// balance
		half y = parent;
		half x = node;
		for(; y != NODE_NIL ;)
		{
			// change parent balance factor
			if(tree[y].right == x)
				tree[y].bactor++;
			else
				tree[y].bactor--;

			if(tree[y].bactor == 0) // stop h not change
				return node;
			// goto endp;

			if(abs(tree[y].bactor) > 1) // need balance
			{
				y = balanceNode(y);
				if(tree[y].bactor == 0) // stop h not change
					return node;
				// goto endp;
			}
			x = y;
			y = tree[x].parent;
		}

// endp:
		// balanceTest(root);
		// display(root, 0, 6);
		return node;
	}

	/// Retrieve rotated node.
	half balanceNode(half z)
	{
		assert(z != NODE_NIL);
		// right subtree rotate to left
		if(tree[z].bactor > 1)
		{
			half y = tree[z].right;
			assert(y != NODE_NIL);
			// big left rotate
			if(tree[y].bactor < 0)
			{
				half x = tree[y].left;
				assert(x != NODE_NIL);
				_avl_printf("big left rotate %u\n", tree[x].key);
				rotate(x);
				rotate(x);
				if(tree[x].bactor == 0)
				{
					tree[y].bactor = 0;
					tree[z].bactor = 0;
				}
				else if(tree[x].bactor == 1)
				{
					tree[y].bactor = 0;
					tree[z].bactor = -1;
				}
				else
				{
					tree[y].bactor = 1;
					tree[z].bactor = 0;
				}
				tree[x].bactor = 0;
				z = x;
			}
			else // small left rotate
			{
				_avl_printf("small left rotate %u\n", tree[y].key);
				rotate(y);
				if(tree[y].bactor == 0)
				{
					tree[y].bactor = -1;
					tree[z].bactor = 1;
				}
				else
				{
					tree[y].bactor = 0;
					tree[z].bactor = 0;
				}
				z = y;
			}
		}
		else // left subtree rotate to right
		{
			half y = tree[z].left;
			assert(y != NODE_NIL);
			// big right rotate
			if(tree[y].bactor > 0)
			{
				half x = tree[y].right;
				assert(x != NODE_NIL);
				_avl_printf("big right rotate %u\n", tree[x].key);
				rotate(x);
				rotate(x);
				if(tree[x].bactor == 0)
				{
					tree[y].bactor = 0;
					tree[z].bactor = 0;
				}
				else if(tree[x].bactor == -1)
				{
					tree[y].bactor = 0;
					tree[z].bactor = 1;
				}
				else
				{
					tree[y].bactor = -1;
					tree[z].bactor = 0;
				}
				tree[x].bactor = 0;
				z = x;
			}
			else // small right rotate
			{
				_avl_printf("small right rotate %u\n", tree[y].key);
				rotate(y);
				if(tree[y].bactor == 0)
				{
					tree[y].bactor = 1;
					tree[z].bactor = -1;
				}
				else
				{
					tree[y].bactor = 0;
					tree[z].bactor = 0;
				}
				z = y;
			}
		}
		return z;
	}

	/// Search node by key.
	half search(T1 key, half node = NODE_NIL)
	{
		if(node == NODE_NIL)
			node = root;
		parent = NODE_NIL;
		while(node != NODE_NIL and key != tree[node].key)
		{
			parent = node;
			if(key < tree[node].key)
				node = tree[node].left;
			else
				node = tree[node].right;
		}
		return node;
	}

	/// Move exist node to root.
	half moveToRoot(half node)
	{
		assert(0);
		if(node == NODE_NIL or node == root)
			return node;
		while(node != root)
			rotate(node);
		return node;
	}

	/// Rotate x (child) on y (parent)
	half rotate(half x)
	{
		if(x == NODE_NIL or x == root)
			return x;
		half y = tree[x].parent;
		half z = tree[y].parent;
		if(tree[y].right == x) // right
		{
			// NOTE 1
			tree[y].right = tree[x].left;
			if(tree[x].left != NODE_NIL)
				tree[tree[x].left].parent = y;
			tree[x].left = y;
		}
		else // left
		{
			// NOTE 2
			tree[y].left = tree[x].right;
			if(tree[x].right != NODE_NIL)
				tree[tree[x].right].parent = y;
			tree[x].right = y;
		}
		// NOTE 3
		if(z == NODE_NIL)
			root = x;
		else if(y == tree[z].left)
			tree[z].left = x;
		else
			tree[z].right = x;
		tree[x].parent = z;
		tree[y].parent = x;
		return x;
		/*
			NOTE 1
			Y R = X L
			X L P = Y
			X L = Y

			NOTE 2
			Y L = X R
			X R P = Y
			X R = Y

			NOTE 3
			Y P == NIL
				root = X
			elif Y == Y P L
				Y P L = X
			else
				Y P R = X
			X P = Y P
			Y P = X
		*/
	}

	/// Balance removing node x.
	half balanceRemove(half x)
	{
		if(x == NODE_NIL) return x;
		half y = tree[x].parent;
		for(; y != NODE_NIL ;)
		{
			// change parent balance
			if(tree[y].right == x)
				tree[y].bactor--;
			else
				tree[y].bactor++;

			// stop, height not change
			if(abs(tree[y].bactor) == 1)
				return x;

			if(abs(tree[y].bactor) > 1)
			{
				y = balanceNode(y);
				if(tree[y].bactor != 0)
					return x;
			}
			x = y;
			y = tree[x].parent;
		}
		return x;
	}

	/// Remove exist node.
	half remove(half y)
	{
		if(y == NODE_NIL) return y;
		_avl_printf("\n==================\n");
		_avl_printf("remove %u | %u h | %u sz\n\n", tree[y].key, getHeightNode(root), size());
		// display(root, 0, 6);
		_avl_printf("\n");
		half x = NODE_NIL;
		if(tree[y].left == NODE_NIL or tree[y].right == NODE_NIL)
		{
			balanceRemove(y);
			if(tree[y].right != NODE_NIL)
				replace(y, tree[y].right);
			else
				replace(y, tree[y].left);
		}
		else
		{
			x = minimum(tree[y].right);
			assert(x != NODE_NIL);
			balanceRemove(x);
			assert(tree[y].right != NODE_NIL);
			if(tree[x].parent != y)
			{
				replace(x, tree[x].right);
				tree[x].right = tree[y].right;
				tree[tree[x].right].parent = x;
			}
			replace(y, x);
			tree[x].left = tree[y].left;
			if(tree[x].left != NODE_NIL)
				tree[tree[x].left].parent = x;
			tree[x].bactor = tree[y].bactor;
		}
		tree[y].parent = NODE_NIL;
		tree[y].left = NODE_NIL;
		tree[y].right = firstAvail;
		tree[y].bactor = 0;
		firstAvail = y;
		totalAvail++;
		// balanceTest(root);
		return y;
	}

	/// Replace y by x.
	void replace(half y, half x)
	{
		half z = tree[y].parent;
		if(z == NODE_NIL)
			root = x;
		else if(y == tree[z].left)
			tree[z].left = x;
		else
			tree[z].right = x;
		if(x != NODE_NIL)
			tree[x].parent = z;
	}

	half minimum(half node)
	{
		if(tree[node].left == NODE_NIL)
			return node;
		return minimum(tree[node].left);
	}

	half maximum(half node)
	{
		if(tree[node].right == NODE_NIL)
			return node;
		return maximum(tree[node].right);
	}

	/// Remove low node, max or min.
	half removeLow()
	{
		half node = NODE_NIL;
		parent = root;
		assert(root != NODE_NIL);

		assert(tree[root].left != NODE_NIL or tree[root].right != NODE_NIL);
		if(tree[root].left != NODE_NIL)
			node = minimum(tree[root].left);
		else
			node = maximum(tree[root].right);

		if(tree[root].left != NODE_NIL)
			replace(node, tree[node].right);
		else
			replace(node, tree[node].left);

		tree[node].parent = NODE_NIL;
		tree[node].left = NODE_NIL;
		tree[node].right = NODE_NIL;
		return node;
	}

	/// Retrieve next node after x.
	half successor(half x)
	{
		if (tree[x].right != NODE_NIL)
			return minimum(tree[x].right);
		half y = tree[x].parent;
		while (y != NODE_NIL and x == tree[y].right)
		{
			x = y;
			y = tree[y].parent;
		}
		return y;
	}

	/// Retrieve previous node before x.
	half predecessor(half x)
	{
		if (tree[x].left != NODE_NIL)
			return maximum(tree[x].left);
		half y = tree[x].parent;
		while (y != NODE_NIL and x == tree[y].left)
		{
			x = y;
			y = tree[y].parent;
		}
		return y;
	}

	node_type& operator [] (uint i)
	{
		assert(i < tree.size());
		return tree[i];
	}

	/// Retrieve node height, recursive.
	uint getHeightNode(half x)
	{
		if(x == NODE_NIL) return 0;
		uint lh = getHeightNode(tree[x].left);
		uint rh = getHeightNode(tree[x].right);
		return MAX(lh, rh) + 1;
	}

	/// Test balance for node x.
	half balanceTestNode(half x)
	{
		if(x == NODE_NIL) return x;
		uint lh = getHeightNode(tree[x].left);
		uint rh = getHeightNode(tree[x].right);
		int k = rh - lh;
		assert(tree[x].bactor == k);
		if(tree[x].bactor != k)
			stdprintf("~~~ balance factor %d %d %u\n", k, tree[x].bactor, tree[x].key);
		/*
		if(k > 1) // small rotate right to left
			rotate(tree[x].right);
		else if(k < -1) // small rotate left to right
			rotate(tree[x].left);
		*/
		return x;
	}

	/// Build tree from sorted keys to increase.
	half build(half node, T1* keys, T2* vals, uint count)
	{
		uint left = 0;
		uint right = count - 1;
		uint mid = right / 2;
		node = insert(keys[mid], vals[mid], node);
		if(count > 1)
		{
			if(mid)
			{
				half left_node = build(node, keys, vals, mid); // left
				tree[node].left = left_node;
			}
			half right_node = build(node, keys + mid + 1, vals + mid + 1, right - mid); // right
			tree[node].right = right_node;
		}
		return node;
	}

	void buildRoot(T1* keys, T2* vals, uint count)
	{
		root = build(root, keys, vals, count);
	}

	/// Test balance of each node, recursive.
	void balanceTest(half node)
	{
		if(node == NODE_NIL)	return;
		balanceTestNode(node);
		balanceTest(tree[node].left);
		balanceTest(tree[node].right);
	}

	/// Display node.
	void displayNode(half node, half shift, half offset)
	{
		if(node == NODE_NIL)
			return;
		stdprintf("  ");
		if(shift)
		{
			uint cn = (shift-1) / offset;
			if(cn)
			{
				stdprintf("%c", 0xB3);
				form(j, cn)
				{
					forn(offset)
					stdprintf(" ");
					if(j < cn-1)
						stdprintf("%c", 0xB3);
				}
			}
			stdprintf("%c", 0xC3);
			forn(offset)
			stdprintf("%c", 0xC4);
		}
		stdprintf(" %u %d\n", tree[node].key, tree[node].bactor);
		return;
	}

	/// Display tree in reverse order traversal R N L, recursive.
	void display(half node, half shift, half offset)
	{
		if(node == NODE_NIL)
			return;
		display(tree[node].right, shift + offset, offset);
		displayNode(node, shift, offset);
		display(tree[node].left, shift + offset, offset);
		return;
	}
};

}

#endif