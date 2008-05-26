void
gimp(void) {
	unsigned int i, n, nx, ny, ny_left, ny_right, nw, nh, sidestackdiv;
	Client *c = nexttiled(clients);

	nx = wax;
	ny = way;
	ny_left = way;
	ny_right = way;
	nw = waw;
	nh = wah;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
                n++;
    if (n <= 2) {
        for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), i++) {
    		c->ismax = False;
    		resize(c, wax, way, waw, wah, False);
    	}
        focusnext(NULL);
        restack();
        return;
    }

	c = nexttiled(clients);

	sidestackdiv = ((n - 1) % 2);
	if (sidestackdiv == 0) {
		sidestackdiv = (n - 1) / 2;
	}
	else {
		sidestackdiv = (n - 1) / 2 + 1;
	}
	if (sidestackdiv < 1) {
		sidestackdiv = 1;
	}
	printf("Hi, we have %d clients\n", n);fflush(stdout);
	printf("Our sidestackdiv is %d\n", sidestackdiv);fflush(stdout);
        
    for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), i++) {
    printf("Hi, positioning client number %d\n", i);fflush(stdout);
	c->ismax = False;
	printf("Hi, c isnt max, i is %d\n", i);fflush(stdout);    
    if(i==n) { /* root */
	    resize(c, wax, way, waw - 2 * c->border, wah - 2 * c->border, False);
        continue;
    }
	if(i == 0) { /* master */
		printf("Doing the master, i is %d\n", i);fflush(stdout);
		nx = wax + waw / 6;
		nw = waw - waw / 3;
        ny = way;
        nh = wah;
	}
	else {  /* tile windows */
    	printf("Doing a client, i is %d\n", i);fflush(stdout);
    	if(i % 2 == 0) {
			printf("Doing an even client, i is %d\n", i);fflush(stdout);
			nx = wax;
            nh = wah / sidestackdiv;        // DIV of the sidestacks num
                                            // determines height of unfocused windows
            nw = waw / 6;                   // hardcode sides-width to 1/6th of full width
            ny = ny_left;
            ny_left = ny_left + nh;
		}
		else {
    		printf("Doing an uneven client, i is %d\n", i);fflush(stdout);
			nx = waw - waw / 6;
            nw = waw / 6;
            nh = wah / (sidestackdiv);
            ny = ny_right;
            ny_right = ny_right + nh;
    	}
	}
	resize(c, nx, ny, nw - 2 * c->border, nh - 2 * c->border, False);
	printf("---------------\n");fflush(stdout);
	}
	focusnext(NULL);
	restack();
}

