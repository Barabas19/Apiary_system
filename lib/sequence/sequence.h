#ifndef SEQENCE_H_
#define SEQENCE_H_

namespace Sequence
{
    enum Status {
        INACTIVE = 0,
        RUNNING = 1,
        FINISHED_SUCCESS = 2,
        FINISHED_FAILED = 9
    };

} // namespace Sequence

#endif // SEQENCE_H_
