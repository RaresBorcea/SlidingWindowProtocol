// Define the node of a queue
typedef struct node {
	msg data;
	struct node *next;
}Node;

// Define the queue structure
typedef struct queue {
	Node *head, *tail;
	int size;
}*Queue;

// Node initialization
Node *initNode(msg data) {
	Node *node = malloc(sizeof(struct node));
	msg node_data = data;
	node->data = node_data;
	node->next = NULL;
	return node;
}

// Free memory for a node
Node *freeNode(Node *node) {
	if(node) {
		free(node);
	}
	return NULL;
}

// Queue initialization
Queue initQueue(msg data) {
	Queue queue = malloc(sizeof(struct queue));
	queue->head = queue->tail = initNode(data);
	queue->size = 1;
	return queue;
}

// Check for an empty queue
int isEmptyQueue(Queue queue) {
	if (queue == NULL || queue->head == NULL || queue->size == 0) {
		return 1;
	} else {
		return 0;
	}
}

// Add node in queue
Queue enqueue(Queue queue, msg data) {
	if(isEmptyQueue(queue)) {
		if(queue == NULL) {
			queue = initQueue(data);
		} else {
			free(queue);
			queue = initQueue(data);
		}
	} else {
		Node *node = initNode(data);
		queue->tail->next = node;
		queue->tail = node;
		queue->size++;
	}
	return queue;
}

// Delete first node of queue
Queue dequeue(Queue queue) {
	if(isEmptyQueue(queue)) {
		return NULL;
	} else {
		Node *temp = queue->head;
		queue->head = queue->head->next;
		queue->size--;
		freeNode(temp);
		return queue;
	}
}

// Value of first node in queue
msg first(Queue queue) {
	msg m;
	memset(&m, 0, sizeof(m));
	if(isEmptyQueue(queue)) {
		return m;
	} else {
		return queue->head->data;
	}
}

// Free queue memory
Queue freeQueue(Queue queue) {
	while(!isEmptyQueue(queue)) {
		queue = dequeue(queue);
	}
	if(queue) {
		free(queue);
	}
	return NULL;
}

// Deleting node 'seq' from queue
Queue dequeueValue(Queue queue, int seq) {
	if(isEmptyQueue(queue)) {
		return NULL;
	} else {
		Node *temp = queue->head;
		if((*((my_pkt*)temp->data.payload)).seq == seq) {
			if(queue->head == queue->tail) {
				queue->tail = queue->tail->next;
			}
			queue->head = queue->head->next;
			queue->size--;
			freeNode(temp);
		} else {
			while(temp->next != NULL && (*((my_pkt*)temp->next->data.payload)).seq != seq) {
				temp = temp->next;
			}
			if(temp->next != NULL) {
				Node *temp2 = temp->next;
				if(temp->next->next == NULL) {
					queue->tail = temp;
				}
				temp->next = temp->next->next;
				queue->size--;
				freeNode(temp2);
			}
		}
		
		return queue;
	}
}