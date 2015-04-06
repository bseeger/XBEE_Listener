/* Description: Javascript (with JQuery and Ajax) to gather and display 
 *             temperature information.  
 * Author: B. Seeger
 * Date: March 2015
 */

$(document).ready( function(){
    updateTemps();
    /* Consideration - switch paradigms to use setTimeout so that 
     * connections won't pile up if something is taking 
     * too long. For now setInterval is probably fine. 
     */
    setInterval(updateTemps,5000);

});

function updateTemps() {
    /* data will look like this  "xx.xx, xx.xx," (yes, ugly trailing 
     * comma for the time being) 
     */
    $.get("data/curval.txt",  function(data) {
        var dateObj = new Date();
        var tmps = data.split(",");

        var tmp1 = parseFloat(tmps[0]);
        var tmp2 = parseFloat(tmps[1]);
         
        var tmp1_S = $("#tmp1"); 
        var tmp2_S = $("#tmp2");

        var tmp1_header_S = $("#tmp1-header");
        var tmp2_header_S = $("#tmp2-header");

        updateTempObj(tmp1_S, tmp1_header_S, tmp1);
        updateTempObj(tmp2_S, tmp2_header_S, tmp2);
        
        $("#timestamp").html("<em>last updated: " + dateObj.toUTCString() + "</em>");
    }); 
}

// updates a temperature div - assumption is that div is of class type 'panel'
function updateTempObj(obj, objHeader, tmp) {
    objHeader.removeClass("panel-success panel-danger panel-primary");
   
    /* Change the background colors if the temperature is 
     * either 80 F or above, or 32 F or less. 
     * This is to warn the user that there is danger of over-heading or frost 
     * in the hoop house. 
     */
    if (tmp >= 80) {
        objHeader.addClass("panel-danger");
    } else if (tmp <= 32) {
        objHeader.addClass("panel-info");
    } else {
        objHeader.addClass("panel-success");
    }

    obj.html(tmp + "&deg;F");
}

