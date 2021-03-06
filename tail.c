                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                o */
	struct node *previous;
	struct node *next;
};

struct tail{
	struct node *first;
	struct node *end;
} tail = {NULL, NULL};

bool tail_is_empty(const struct tail tail)
{//return true if tail is empty
	return ((tail.first == NULL) && (tail.end == NULL))? true: false;
}

void node_init(struct node *ptr, const int id, const pid_t pid, const char *command, enum status status)
{//Init node
	ptr->id = id;
	ptr->pid = pid;
	ptr->command = strdup(command);
	ptr->status = status;
	ptr->next = ptr->previous = NULL;
}

int tail_add(struct tail *tail, const pid_t pid, char *command, enum status status)
{//add a element into tail
	struct node *new = malloc(sizeof(struct node));
	if (new == NULL) {
		perror("cola_add: malloc");	
		exit(EXIT_FAILURE);
		return -1;//NO MEMORY
	}
	
	int id = 1;
	if (tail->first != NULL) {//Tail isn't empty
		id = (tail->end)->id + 1;
		node_init(new, id, pid, command, status);//Init node
		new->previous = tail->end;
		(tail->end)->next = new;
	}
	else{//Tail is empty
		node_init(new, id, pid, command, status);//Init node
		tail->first = new;
	}
	
	tail->end = new;
	return 0;
}

struct node *tail_delete(struct tail *tail, const int id)
{//Delete a element of tail
	if (tail->first != NULL) {
		struct node *auxNext, *aux = tail->first;
		while ((aux->next != NULL) && (aux->id != id))/* la segunda es la comparacion */
			aux = aux->next;
		if (aux->id == id) {//It's the element to delete
			if (aux != tail->end)
				(aux->next)->previous = aux->previous;
			else
				tail->end = aux->previous;
			if (aux != tail->first)
				(aux->previous)->next = aux->next;
			else
				tail->first = aux->next;
			if (aux->command != NULL)
				free(aux->command);
			auxNext = aux->next;
			free(aux);
			return auxNext;
		}
		return 0;
	}

	return NULL;//The element doesn't exist
}

void tail_delete_whole(struct tail *tail)
{
	if (tail->first == NULL)
		return;
		
	struct node *aux2, *aux1 = tail->first;
	
	while (aux1 != NULL) {
		aux2 = aux1;
		aux1 = aux1->next;
		if (aux2->command != NULL)
			free(aux2->command);
		free(aux2);
	}
}

struct node *tail_find(const struct tail tail, const int id)
{
	if (tail.first != NULL) {//Tail isn't empty
		struct node *aux = tail.first;
		while ((aux != NULL) && (id < aux->id))
			aux = aux->next;
		if ((aux != NULL) && (aux->id == id))
			return aux;
	}
	
	/* The element doesn't exist */
	return NULL;
}

void tail_print(const struct tail tail)
{//Print whole tail
	if (tail.first != NULL) {//Tail isn't empty
		struct node *aux = tail.first;
		while (aux != NULL) {
			printf("[%d] %s %s\n", aux->id, status2string(aux->status), aux->command);
			aux = aux->next;	
		}
		return;
	}
	
	/* Tail is empty */
	printf("Tail is empty\n");
}


int main(void)
{
//	struct tail tail = {NULL, NULL};
/*
	if (tail_is_empty(tail))
		printf("Tail is empty\n");
	else
		printf("Tail isn't empty\n");
*/	
	tail_add(&tail);
	tail_add(&tail);
	tail_add(&tail);
	tail_delete(&tail, 1);
//	tail_delete(&tail, 2);
	tail_add(&tail);
	tail_delete(&tail, 3);
//	tail_print(tail);
	tail_print(tail);
	struct node *aux;
	aux = tail_find(tail, 3);
	if (aux == NULL)
		printf("Not found: 3\n");
	else
		printf("Find: %d\n", aux->id);
	aux = tail_find(tail, 2);
	if (aux != NULL)
		printf("Find: %d\n", aux->id);
		
	
	return 0;
}
