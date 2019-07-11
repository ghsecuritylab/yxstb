#ifndef IPConflictSetting_h
#define IPConflictSetting_h

#ifdef __cplusplus

class IPConflictSetting {
public:
    IPConflictSetting();
    ~IPConflictSetting();

    IPConflictSetting& operator = (const IPConflictSetting& rhs);

    void setReplyTime(int replyTime);
    const int getReplyTime() const;

    void setConflictTime(int time);
    const int getConflictTime() const;

    void setUnconflictTime(int time);
    const int getUnconflictTime() const;

private:
    int mReplyTime; //ARP应答时间
    int mConflictTime; //冲突时发包间隔时间
    int mUnconflictTime; //不冲突时发包间隔时间
};

#endif

#endif
