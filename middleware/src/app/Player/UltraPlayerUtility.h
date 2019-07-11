#ifndef _UltraPlayerUtility_H_
#define _UltraPlayerUtility_H_

#ifdef __cplusplus

namespace Hippo {

class Program;
class UltraPlayer;
class UltraPlayerClient;
class BrowserPlayerReporter;

class UltraPlayerUtility {
public:
    UltraPlayerUtility();
    ~UltraPlayerUtility();

    static UltraPlayer *createPlayerByProgram(UltraPlayerClient *, BrowserPlayerReporter *, Program *);
    static UltraPlayer *createPlayerByType(int);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerUtility_H_
