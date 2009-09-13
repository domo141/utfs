#if 0 /*
sed -n '/[ ]VERSION / { s/.*VERSION *"/version=/; s|".*||;p; }
	/[ ]PROTVER / { s!.*PROTVER *!protver=!p }' $0
exit 0
*/
#endif
/*
 * Created: Sat Jan 19 16:46:57 EET 2008 too
 * Last modified: Sun 13 Sep 2009 21:57:39 EEST too
 */

#ifndef VERSION_H
#define VERSION_H

#define VERSION "1.952"
#define PROTVER 4

#endif /* VERSION_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
