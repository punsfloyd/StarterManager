# StarterManager
This application is developed using the starter resource manager interface for QNX 6.5.0 OS for TATA project.

This application controls the startup sequence of services running on X104_HIGH project based upon the last user mode context.
The application also monitors the health of serivces by checking their state whether RUN or STOP.
In case any service state goes to STOP(crashed), it notifies about it to the system.
