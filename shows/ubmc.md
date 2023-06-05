. define-style default
. save-style


. define-style title
. color 1 1 1
. font .3 center bold
. line-height .69
. save-style

. define-style subtitle
. font .1
. save-style

. define-style body
. font .15 left
. line-height .9
. save-style

. define-image oss oss-wheel

. # xd
. font .5

# x
. using xd
. image oss
. color .4 .4 1
. font .4

# x
. bg 0 0.682 0.937
. style title
. y .1
Dashboard
and 
Monitoring

# x
. style subtitle
. bg 0.063 0.325 0.537
Dashboard and monitoring

. style body
. y .2
- Health
- Live chart
- Summary of attributes
. font .1
. line-height 1
   * Consider support use cases
. style body
- Links

# x
. style title
. y .3
. bg 0 0.682 0.937
Settings

# x
. style subtitle
. bg 0.063 0.325 0.537
Settings

. style body
. y .15
. font .13
- Events: email and HTTP
- Time, NTP, Ethernet, DNS,
   SSH, IPMI,
- Events service and listeners
. font .1
. line-height 1
  * HTTP demo
. style body
. font .13
- BMC power cold/warm reset

# x
. style title
. y .3
. bg 0 0.682 0.937
Monitoring

# x
. style subtitle
. bg 0.063 0.325 0.537
Monitoring

. style body
. y .2
- Sensor tables
- Table features
. font .1
. line-height 1
  * Sorting, search filtering, ...
  * Hidden columns
  * Remembers settings after refresh
. style body

# x
. style title
. y .3
. bg 0 0.682 0.937
Device Tree

# x
. style title
. y .2
. font .28
. bg 0 0.682 0.937
User 
Management

# x
. style subtitle
. bg 0.063 0.325 0.537
User management

. style body
. y .2
- Change users
- User authorization
. font .1
. line-height 1
  * Administrator - All permissions
  * Operator - Does not manage users
  * User - Read-only
. style body

# x
. style title
. y .3
. bg 0 0.682 0.937
Other

# x
. style body
. y .2
. bg 0.063 0.325 0.537
- Mobile 
- Themes: light&
. y .349
. color 0 0 0 
                             dark
. style body
. y .5
- OSS branding 
  * Footer/About

# x
. y .03
. font .08
. line-height 1.2
. bg 0.769 0.125 0.196
In progress:
- Software Update, Factory Reset,
  Audit Log, Service Report,
  Boot order, Location, 
  System Power (head unit)
- LED settings 
   * Requires tuning to real hardware
- Device tree usability
- Deep link bookmarks (a nice browser feature)

# x
. font .55 center
. y .2
. bg 0.063 0.325 0.537
Thanks!
