function maxSpeed(maxForward, maxBackward, maxSideward, maxRotation)
maxY = maxSideward
maxRot = maxRotation;


axis equal;
hold all;
for x=-100:5:100
    for y=-100:5:100
        for yaw=-100:5:100
            if x >= 0
                maxX = maxForward;
            else
                maxX = maxBackward;
            end
            length = sqrt(x*x + y*y);
            tau = atan(maxX/maxY * y/x);
            mx = maxX * cos(tau);
            my = maxY * sin(tau);
            maxLength = sqrt(mx*mx + my*my);
            
            mx = maxX;
            my = maxY;
            cyaw = yaw;
            if ((length*length)/(maxLength*maxLength)+(yaw*yaw)/(maxRot*maxRot) > 1)
                phi = atan(maxLength/maxRot * yaw/length);
                cl   = maxLength * cos(phi);
                cyaw = maxRot * sin(phi);
                lengthFactor = cl/length;
                mx = maxX * lengthFactor;
                my = maxY * lengthFactor;
            end
            if ((x*x)/(mx*mx)+(y*y)/(my*my) > 1)
                phi = atan(maxX/maxY * y/x);
                if x >= 0
                    cx = mx * cos(phi);
                    cy = my * sin(phi);
                else
                    cx = -mx * cos(phi);
                    cy = -my * sin(phi);
                end
                plot3(cy, cx, cyaw, 'o');
                %plot3(y, x, yaw, 'x');
                %line([y cy], [x cx], [yaw cyaw]);
                
            else
               % plot3(y, x, cyaw, 'o');
            end
        end
    end
end