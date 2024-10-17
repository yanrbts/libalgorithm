/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avltree.h>

static int counter;
static int test_array[1000];

static int int_compare(void *vlocation1, void *vlocation2) {
    int *location1;
	int *location2;

	location1 = (int *)vlocation1;
	location2 = (int *)vlocation2;

	if (*location1 < *location2) {
		return -1;
	} else if (*location1 > *location2) {
		return 1;
	} else {
		return 0;
	}
}

static AVLTree *create_tree(void) {
    AVLTree *tree;
    int i;

    tree = avlTreeNew((AVLTreeCompareFunc)int_compare);
    for (i=0; i<1000; ++i) {
		test_array[i] = i;
		avlTreeInsert(tree, &test_array[i], &test_array[i]);
	}

	return tree;
}

static int find_subtree_height(AVLTreeNode *node) {
	AVLTreeNode *left_subtree;
	AVLTreeNode *right_subtree;
	int left_height, right_height;

	if (node == NULL) {
		return 0;
	}

	left_subtree = avlTreeNodeChild(node, AVL_TREE_NODE_LEFT);
	right_subtree = avlTreeNodeChild(node, AVL_TREE_NODE_RIGHT);
	left_height = find_subtree_height(left_subtree);
	right_height = find_subtree_height(right_subtree);

	if (left_height > right_height) {
		return left_height + 1;
	} else {
		return right_height + 1;
	}
}

static int validate_subtree(AVLTreeNode *node) {
    AVLTreeNode *left_node, *right_node;
	int left_height, right_height;
	int *key;

    if (node == NULL) return 0;

    left_node = avlTreeNodeChild(node, AVL_TREE_NODE_LEFT);
    right_node = avlTreeNodeChild(node, AVL_TREE_NODE_RIGHT);

    if (left_node != NULL) TEST_ASSERT(avlTreeNodeParent(left_node) == node);
    if (right_node != NULL) TEST_ASSERT(avlTreeNodeParent(right_node) == node);

    left_height = validate_subtree(left_node);
    key = (int *)avlTreeNodeKey(node);
    TEST_ASSERT(*key > counter);
    counter = *key;

    right_height = validate_subtree(right_node);
    TEST_ASSERT(avlTreeSubtreeHeight(left_node) == left_height);
    TEST_ASSERT(avlTreeSubtreeHeight(right_node) == right_height);

    TEST_ASSERT(left_height - right_height < 2 &&
	       right_height - left_height < 2);

    /* Calculate the height of this node */
	if (left_height > right_height) {
		return left_height + 1;
	} else {
		return right_height + 1;
	}
}

static void validate_tree(AVLTree *tree)
{
	AVLTreeNode *root_node;
	int height;

	root_node = avlTreeRootNode(tree);

	if (root_node != NULL) {
		height = find_subtree_height(root_node);
		TEST_ASSERT(avlTreeSubtreeHeight(root_node) == height);
	}

	counter = -1;
	validate_subtree(root_node);
}

static void test_avlTreeFree(void) {
	AVLTree *tree;

	/* Try freeing an empty tree */
	tree = avlTreeNew((AVLTreeCompareFunc)int_compare);
	avlTreeFree(tree);

	/* Create a big tree and free it */
	tree = create_tree();
	avlTreeFree(tree);
}

static void test_avlTreeNew(void) {
    AVLTree *tree;

    tree = avlTreeNew((AVLTreeCompareFunc)int_compare);
    TEST_ASSERT_NOT_EQUAL(NULL, tree);
    TEST_ASSERT(avlTreeRootNode(tree) == NULL);
    TEST_ASSERT(avlTreeNumEntries(tree) == 0);

    avlTreeFree(tree);
}

static void test_avlTreeChild(void) {
    AVLTree *tree;
	AVLTreeNode *root;
	AVLTreeNode *left;
	AVLTreeNode *right;
	int values[] = { 1, 2, 3 };
	int *p;
	int i;

    tree = avlTreeNew((AVLTreeCompareFunc)int_compare);

	for (i=0; i<3; ++i) {
		avlTreeInsert(tree, &values[i], &values[i]);
	}

    root = avlTreeRootNode(tree);
	p = avlTreeNodeValue(root);
	TEST_ASSERT(*p == 2);

    left = avlTreeNodeChild(root, AVL_TREE_NODE_LEFT);
	p = avlTreeNodeValue(left);
	TEST_ASSERT(*p == 1);

	right = avlTreeNodeChild(root, AVL_TREE_NODE_RIGHT);
	p = avlTreeNodeValue(right);
	TEST_ASSERT(*p == 3);

    TEST_ASSERT(avlTreeNodeChild(root, 10000) == NULL);
    TEST_ASSERT(avlTreeNodeChild(root, 2) == NULL);

    avlTreeFree(tree);
}

static void test_avlTreeLookup(void) {
    AVLTree *tree;
	int i;
	int *value;

    tree = create_tree();
    for (i = 0; i < 1000; ++i) {
        value = avlTreeLookup(tree, &i);
        TEST_ASSERT(value != NULL);
        TEST_ASSERT(*value == i);
    }

    i = -1;
	TEST_ASSERT(avlTreeLookup(tree, &i) == NULL);
	i = 1000 + 1;
	TEST_ASSERT(avlTreeLookup(tree, &i) == NULL);
	i = 8724897;
	TEST_ASSERT(avlTreeLookup(tree, &i) == NULL);

    avlTreeFree(tree);
}

static void test_avlTreeRemove(void) {
    AVLTree *tree;
	int i;
	int x, y, z;
	int value;
	unsigned int expected_entries;

	tree = create_tree();

    expected_entries = 1000;
    for (x=0; x<10; ++x) {
        for (y=0; y<10; ++y) {
            for (z=0; z<10; ++z) {
                value = z * 100 + (9 - y) * 10 + x;
                TEST_ASSERT(avlTreeRemove(tree, &value) != 0);
                validate_tree(tree);
                expected_entries -= 1;
                TEST_ASSERT(avlTreeNumEntries(tree) == expected_entries);
            }
        }
    }

    /* All entries removed, should be empty now */
	TEST_ASSERT(avlTreeRootNode(tree) == NULL);

    avlTreeFree(tree);
}

static void test_avlTreeToArray(void) {
    AVLTree *tree;
	int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
	int sorted[]  = { 4, 8, 15, 16, 23, 30, 42, 50, 89, 99 };
	unsigned int num_entries = sizeof(entries) / sizeof(int);
	unsigned int i;
	int **array;

    tree = avlTreeNew((AVLTreeCompareFunc)int_compare);
    for (i = 0; i < num_entries; i++) {
        avlTreeInsert(tree, &entries[i], NULL);
    }
    TEST_ASSERT(avlTreeNumEntries(tree) == num_entries);

    array = (int**)avlTreeToArray(tree);
    for (i = 0; i < num_entries; ++i) {
		TEST_ASSERT(*array[i] == sorted[i]);
	}
    free(array);

    array = (int **)avlTreeToArray(tree);
    TEST_ASSERT(array != NULL);
    validate_tree(tree);

    avlTreeFree(tree);
}

void test_avltree(void) {
    test_avlTreeFree();
    test_avlTreeNew();
    test_avlTreeChild();
    test_avlTreeLookup();
    test_avlTreeRemove();
    test_avlTreeToArray();
}