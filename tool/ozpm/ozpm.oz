functor
import
   Global(args:Args)
   ActionInteractive
   ActionInstall(run)
   ActionList(run)
   ActionInfo(run)
   ActionCreate(run)
   ActionHelp(run)
   ActionRemove(run)
define
   case Args.action
   of list then % list all installed packages
      {ActionList.run}
   [] info then % view the contents of a package
      {ActionInfo.run}
   [] create then % create a new package
      {ActionCreate.run}
   [] install then % install/update a specified package
      {ActionInstall.run}
   [] check then % check installed packages integrity and rebuilds if necessary
      skip
   [] remove then % removes a package
      {ActionRemove.run}
   [] help then % display some help
      {ActionHelp.run}
   [] interactive then % start the application in interactive mode
      {New ActionInteractive.'class' run _}
   end
end
