functor
import
   DPPane(siteStatistics) at 'x-oz://boot/DPPane'
   DPB at 'x-oz://boot/DPB'
export
   sitesDict:SitesDict
   sites:SiteInfo
define
   {Wait DPB}

   proc{SiteFnC Y X Z}
      NS = if ({Y.4 member(X.siteid $)}) then Y.1
           else X|Y.1 end
      NR = Y.2 + 1
      AS = if (X.sent >0) orelse (X.received > 0) then
              X|Y.3 else  Y.3 end
/*
      SI = case {TOS X.type} of
              connected then {AdjoinAt Y.5 remote (Y.5).remote+1 $}
           elseof unconnected then
              {AdjoinAt Y.5 unconnected (Y.5).unconnected+1 $}
           elseof dead then {AdjoinAt Y.5 dead (Y.5).dead+1 $}
           else raise unknown(site_spec({TOS X.type})) end
           end
*/
   in
      Z = r(NS NR AS Y.4 _)
   end

/*
   local
      REMOTE_SITE           = 1
%      VIRTUAL_SITE          = 2
%      VIRTUAL_INFO          = 4
      CONNECTED             = 8
      PERM_SITE             = 16
      SECONDARY_TABLE_SITE  = 32
      MY_SITE                 = 64
%      GC_MARK                = 128
   in
      fun{TOS T}
         if T div MY_SITE == 1 then unconnected
         elseif T div SECONDARY_TABLE_SITE == 1  then dead
         elseif T div PERM_SITE == 1  then dead
         elseif T div CONNECTED == 1 then  connected
         elseif T == REMOTE_SITE then unconnected
         elseif T == 0 then unconnected
         else raise unknown_site(T) end hell
         end
      end
   end
*/

   class Site
      feat string
         key
         GUI
         sd
      attr
         col
         own
         bor
         act
         gen
         sent
         received
         lastRTT
         deltaSent
         deltaReceived
         graphKey

      meth init(S G SD)
         self.key = S.siteid
         self.string = S
         self.GUI = G
         self.sd = SD
         own <- false
         bor <- false
         act <- false
         col <- none
         gen <- 0
         sent <- 0
         received <- 0
         lastRTT <- {IntToFloat ~1}
         deltaSent <- 0
         deltaReceived <- 0
      end

      meth delete
         skip
      end

      meth getCol(who:W $)
         if @col == none then
            col <- {self.sd getACol($)}
            {self.GUI.ssites setColour(key:self.key fg:@col bg:lightgrey)}
         else skip end
         W <- true
         @col
      end

      meth retCol(who:W)
         @W = true
         W <- false
         if @own andthen @bor andthen @act then
            {self.GUI.ssites setColour(key:self.key fg:black bg:lightgrey)}
            {self.sd retACol(@col)}
            col <- none
         else skip end
      end
      meth getGen($) @gen end
      meth setGen(G) gen<-G end
      meth getSent($) @deltaSent end
      meth sent(G) sent <- @sent + G deltaSent <- G end
      meth getReceived($) @deltaReceived end
      meth received(G) received <- @received + G deltaReceived <- G end
      meth setLastRTT(RTT) lastRTT<-{IntToFloat RTT} end
      meth getLastRTT($) @lastRTT end
      meth setGraphKey(K) graphKey<-K end
      meth getGraphKey($) @graphKey end
   end

   class SitesDict
      feat
         Sites
         GUI
      attr
         NrOfSites
         Colours
      meth init(C G)
         self.Sites = {NewDictionary}
         NrOfSites <- 0
         Colours <- C
         self.GUI = G
      end

      meth getSite(S $)
         {Dictionary.get self.Sites S}
      end

      meth newSite(S AS)
         SS={New Site init(S self.GUI self)}in
         {Dictionary.put self.Sites S.siteid SS}
         NrOfSites <- @NrOfSites + 1
         AS = site(key:S.siteid text:S.ip#":"#S.port)
      end

      meth removeSite(S AS)
         AS = {Dictionary.get self.Sites S $}
         {AS delete}
         {Dictionary.remove self.Sites S}
         NrOfSites <- @NrOfSites - 1
      end

      meth getNrOS($) @NrOfSites end
      meth getKeys($) {Dictionary.keys self.Sites} end
      meth member(S A) A = {Dictionary.member self.Sites S} end


      meth getACol($)
         S = @Colours in
         Colours <- S.2
         S.1
      end

      meth retACol(C) Colours <- C|@Colours  end

      meth getTypes($)
         L = {Dictionary.entries self.Sites}
      in
         {Map L proc{$ X Y} {X.2 getType(Y)} end}
      end
   end

   class SiteInfo
      prop locking
      feat
         GUI
         sd
      attr
         ActiveSites
         Generation

      meth Fetch(S)
         S = {DPPane.siteStatistics}
      end

      meth init(SD)
         self.sd = SD
         Generation <- 0
         ActiveSites <- nil
      end
      meth setGui(G)
         self.GUI = G
/*
         {self.GUI.snumber addGraph(key:dead        col:red    stp:'' val:0.0)}
         {self.GUI.snumber addGraph(key:unconnected col:black  stp:'' val:0.0)}
         {self.GUI.snumber addGraph(key:remote      col:yellow stp:'' val:0.0)}
*/
      end

      meth Update(active:AS types:T nr:NS)
         SI  =  SiteInfo,Fetch($) % Sitestatistics
         DS % Sites to be deleted
         % r(stats_for_new_sites nof_elements_in_sitestatistics
         %   stats_for_sending/receiving_sites sitedictionary
         %   nof_dead/connected/unconnected)
         r(CS !NS !AS self.sd !T) =
          {FoldL SI SiteFnC r(nil 0 nil self.sd r(remote:0 unconnected:0 dead:0)) $}
      in
         if(NS - {Length CS} == {self.sd getNrOS($)}) then
            DS = nil
         else
            OS = {self.sd getKeys($)}
         in
            DS = {Filter OS fun{$ Z}
                               {All SI fun{$ X}
                                          X.siteid \= Z
                                       end}
                            end}
         end
         {self.GUI.ssites deleteSite(DS)}
         {self.GUI.ssites addSite({Map CS proc{$ X Y}
                                             {self.sd newSite(X Y)} end})}

         {Map DS proc{$ X Y} {self.sd removeSite(X Y)} end _}
      end

      meth display
         AS
      in
         lock
            Generation<-(@Generation + 1) mod 100
            SiteInfo,Update(active:AS types:_ nr:_)
            SiteInfo,activeSites(AS)
%           SiteInfo,countingSites(T)
         end
      end

      meth activeSites(AS)
         G  =  @Generation
         GG = (@Generation + 1) mod 100
%        GGG = (@Generation + 99) mod 100
      in
         {ForAll AS
          proc{$ S}
             if  {Some @ActiveSites
                  fun{$ X}
                     if  X.key == S.siteid then
                        {X setGen(G)}
                        {X sent(S.sent)}
                        {X received(S.received)}
                        {X setLastRTT(S.lastRTT)}
                        true
                     else false end
                  end}
             then  skip
             else
                Si = {self.sd getSite(S.siteid $)}
                Id  = {NewName}
                Col= {Si getCol(who:act $)}
             in
                ActiveSites <- Si | @ActiveSites
                {Si setGen(G)}
                {Si sent(S.sent)}
                {Si received(S.received)}
                {Si setLastRTT(S.lastRTT)}
                {Si setGraphKey(Id)}
                {self.GUI.sactivity addGraph(key:Id col:Col stp:'' val:0.0)}
                {self.GUI.srtt addGraph(key:Id col:Col stp:'' val:0.0)}
             end
          end}
         local TL ListRTT in
            TL={FoldL @ActiveSites
                proc{$ X SS Y}
                   if {SS getGen($)} == GG then
                      ActiveSites <- {List.subtract
                                      @ActiveSites SS}
                      {SS retCol(who:act)}
                      {self.GUI.sactivity
                       rmGraph(key:{SS getGraphKey($)})}
                      Y = X
                   elseif  {SS getGen($)} == G then
                      Y={IntToFloat
                         {SS getSent($)}+
                         {SS getReceived($)}}#{SS getGraphKey($)}|X
                   else
                      Y=0.0#{SS getGraphKey($)} |X
                   end
                end
                nil}
            {self.GUI.sactivity display(TL)}
            ListRTT={FoldR @ActiveSites fun{$ S Ls}
                                           LastRTT={S getLastRTT($)}
                                        in
                                           if LastRTT>=0.0 then
                                              LastRTT#{S getGraphKey($)}|Ls
                                           else
                                              Ls
                                           end
                                        end
                     nil}
            {self.GUI.srtt display(ListRTT)}
         end
      end

/*
      meth countingSites(SI)
         {self.GUI.snumber display({FoldL [dead unconnected remote]
                                    proc{$ X Y Z}
                                       V = X.1 + X.2.Y
                                    in
                                       Z = r(V X.2 {IntToFloat V}#Y|X.3)
                                    end
                                    r(0 SI nil)}.3)}
      end
*/
   end
end
